/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define ATRACE_TAG (ATRACE_TAG_POWER | ATRACE_TAG_HAL)
#define LOG_TAG "android.hardware.power@1.2-service.nubia_msm8998-libperfmgr"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/strings.h>
#include <android-base/stringprintf.h>

#include <mutex>

#include <utils/Log.h>
#include <utils/Trace.h>

#include "Power.h"
#include "power-helper.h"

/* RPM runs at 19.2Mhz. Divide by 19200 for msec */
#define RPM_CLK 19200

extern struct stat_pair rpm_stat_map[];

namespace android {
namespace hardware {
namespace power {
namespace V1_2 {
namespace implementation {

using ::android::hardware::power::V1_0::Feature;
using ::android::hardware::power::V1_0::PowerStatePlatformSleepState;
using ::android::hardware::power::V1_0::Status;
using ::android::hardware::power::V1_1::PowerStateSubsystem;
using ::android::hardware::power::V1_1::PowerStateSubsystemSleepState;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;

Power::Power() :
        mHintManager(HintManager::GetFromJSON("/vendor/etc/powerhint.json")),
        mInteractionHandler(mHintManager),
        mVRModeOn(false),
        mSustainedPerfModeOn(false),
        mEncoderModeOn(false) {
    mInteractionHandler.Init();

    std::string state = android::base::GetProperty(kPowerHalStateProp, "");
    if (state == "VIDEO_ENCODE") {
        ALOGI("Initialize with VIDEO_ENCODE on");
        mHintManager->DoHint("VIDEO_ENCODE");
        mEncoderModeOn = true;
    } else if (state ==  "SUSTAINED_PERFORMANCE") {
        ALOGI("Initialize with SUSTAINED_PERFORMANCE on");
        mHintManager->DoHint("SUSTAINED_PERFORMANCE");
        mSustainedPerfModeOn = true;
    } else if (state == "VR_MODE") {
        ALOGI("Initialize with VR_MODE on");
        mHintManager->DoHint("VR_MODE");
        mVRModeOn = true;
    } else if (state == "VR_SUSTAINED_PERFORMANCE") {
        ALOGI("Initialize with SUSTAINED_PERFORMANCE and VR_MODE on");
        mHintManager->DoHint("VR_SUSTAINED_PERFORMANCE");
        mSustainedPerfModeOn = true;
        mVRModeOn = true;
    } else {
        ALOGI("Initialize PowerHAL");
    }

    state = android::base::GetProperty(kPowerHalAudioProp, "");
    if (state == "LOW_LATENCY") {
        ALOGI("Initialize with AUDIO_LOW_LATENCY on");
        mHintManager->DoHint("AUDIO_LOW_LATENCY");
    }
}

// Methods from ::android::hardware::power::V1_0::IPower follow.
Return<void> Power::setInteractive(bool /* interactive */)  {
    return Void();
}

Return<void> Power::powerHint(PowerHint_1_0 hint, int32_t data) {
    if (!isSupportedGovernor()) {
        return Void();
    }

    switch(hint) {
        case PowerHint_1_0::INTERACTION:
            if (mVRModeOn || mSustainedPerfModeOn) {
                ALOGV("%s: ignoring due to other active perf hints", __func__);
            } else {
                mInteractionHandler.Acquire(data);
            }
            break;
        case PowerHint_1_0::VIDEO_ENCODE:
            if (mVRModeOn || mSustainedPerfModeOn) {
                ALOGV("%s: ignoring due to other active perf hints", __func__);
                break;
            }
            ATRACE_BEGIN("video_encode");
            if (mVRModeOn || mSustainedPerfModeOn) {
                ALOGV("%s: ignoring due to other active perf hints", __func__);
            } else {
                if (data) {
                    // Hint until canceled
                    ATRACE_INT("video_encode_lock", 1);
                    mHintManager->DoHint("VIDEO_ENCODE");
                    ALOGD("VIDEO_ENCODE ON");
                    if (!android::base::SetProperty(kPowerHalStateProp, "VIDEO_ENCODE")) {
                        ALOGE("%s: could not set powerHAL state property to VIDEO_ENCODE", __func__);
                    }
                    mEncoderModeOn = true;
                } else {
                    ATRACE_INT("video_encode_lock", 0);
                    mHintManager->EndHint("VIDEO_ENCODE");
                    ALOGD("VIDEO_ENCODE OFF");
                    if (!android::base::SetProperty(kPowerHalStateProp, "")) {
                        ALOGE("%s: could not clear powerHAL state property", __func__);
                    }
                    mEncoderModeOn = false;
                }
            }
            ATRACE_END();
            break;
        case PowerHint_1_0::SUSTAINED_PERFORMANCE:
            if (data && !mSustainedPerfModeOn) {
                ALOGD("SUSTAINED_PERFORMANCE ON");
                if (!mVRModeOn) { // Sustained mode only.
                    mHintManager->DoHint("SUSTAINED_PERFORMANCE");
                    if (!android::base::SetProperty(kPowerHalStateProp, "SUSTAINED_PERFORMANCE")) {
                        ALOGE("%s: could not set powerHAL state property to SUSTAINED_PERFORMANCE", __func__);
                    }
                } else { // Sustained + VR mode.
                    mHintManager->EndHint("VR_MODE");
                    mHintManager->DoHint("VR_SUSTAINED_PERFORMANCE");
                    if (!android::base::SetProperty(kPowerHalStateProp, "VR_SUSTAINED_PERFORMANCE")) {
                        ALOGE("%s: could not set powerHAL state property to VR_SUSTAINED_PERFORMANCE", __func__);
                    }
                }
                mSustainedPerfModeOn = true;
            } else if (!data && mSustainedPerfModeOn) {
                ALOGD("SUSTAINED_PERFORMANCE OFF");
                mHintManager->EndHint("VR_SUSTAINED_PERFORMANCE");
                mHintManager->EndHint("SUSTAINED_PERFORMANCE");
                if (mVRModeOn) { // Switch back to VR Mode.
                    mHintManager->DoHint("VR_MODE");
                    if (!android::base::SetProperty(kPowerHalStateProp, "VR_MODE")) {
                        ALOGE("%s: could not set powerHAL state property to VR_MODE", __func__);
                    }
                } else {
                    if (!android::base::SetProperty(kPowerHalStateProp, "")) {
                        ALOGE("%s: could not clear powerHAL state property", __func__);
                    }
                }
                mSustainedPerfModeOn = false;
            }
            break;
        case PowerHint_1_0::VR_MODE:
            if (data && !mVRModeOn) {
                ALOGD("VR_MODE ON");
                if (!mSustainedPerfModeOn) { // VR mode only.
                    mHintManager->DoHint("VR_MODE");
                    if (!android::base::SetProperty(kPowerHalStateProp, "VR_MODE")) {
                        ALOGE("%s: could not set powerHAL state property to VR_MODE", __func__);
                    }
                } else { // Sustained + VR mode.
                    mHintManager->EndHint("SUSTAINED_PERFORMANCE");
                    mHintManager->DoHint("VR_SUSTAINED_PERFORMANCE");
                    if (!android::base::SetProperty(kPowerHalStateProp, "VR_SUSTAINED_PERFORMANCE")) {
                        ALOGE("%s: could not set powerHAL state property to VR_SUSTAINED_PERFORMANCE", __func__);
                    }
                }
                mVRModeOn = true;
            } else if (!data && mVRModeOn) {
                ALOGD("VR_MODE OFF");
                mHintManager->EndHint("VR_SUSTAINED_PERFORMANCE");
                mHintManager->EndHint("VR_MODE");
                if (mSustainedPerfModeOn) { // Switch back to sustained Mode.
                    mHintManager->DoHint("SUSTAINED_PERFORMANCE");
                    if (!android::base::SetProperty(kPowerHalStateProp, "SUSTAINED_PERFORMANCE")) {
                        ALOGE("%s: could not set powerHAL state property to SUSTAINED_PERFORMANCE", __func__);
                    }
                } else {
                    if (!android::base::SetProperty(kPowerHalStateProp, "")) {
                        ALOGE("%s: could not clear powerHAL state property", __func__);
                    }
                }
                mVRModeOn = false;
            }
            break;
        case PowerHint_1_0::LAUNCH:
            ATRACE_BEGIN("launch");
            if (mVRModeOn || mSustainedPerfModeOn) {
                ALOGV("%s: ignoring due to other active perf hints", __func__);
            } else {
                if (data) {
                    // Hint until canceled
                    ATRACE_INT("launch_lock", 1);
                    mHintManager->DoHint("LAUNCH");
                    ALOGD("LAUNCH ON");
                } else {
                    ATRACE_INT("launch_lock", 0);
                    mHintManager->EndHint("LAUNCH");
                    ALOGD("LAUNCH OFF");
                }
            }
            ATRACE_END();
            break;
        default:
            break;

    }
    return Void();
}

Return<void> Power::setFeature(Feature /*feature*/, bool /*activate*/)  {
    //Nothing to do
    return Void();
}

Return<void> Power::getPlatformLowPowerStats(getPlatformLowPowerStats_cb _hidl_cb) {

    hidl_vec<PowerStatePlatformSleepState> states;
    uint64_t stats[MAX_PLATFORM_STATS * MAX_RPM_PARAMS] = {0};
    uint64_t *values;
    struct PowerStatePlatformSleepState *state;
    int ret;

    states.resize(PLATFORM_SLEEP_MODES_COUNT);

    ret = extract_platform_stats(stats);
    if (ret != 0) {
        states.resize(0);
        goto done;
    }

    /* Update statistics for XO_shutdown */
    state = &states[RPM_MODE_XO];
    state->name = "XO_shutdown";
    values = stats + (RPM_MODE_XO * MAX_RPM_PARAMS);

    state->residencyInMsecSinceBoot = values[1];
    state->totalTransitions = values[0];
    state->supportedOnlyInSuspend = false;
    state->voters.resize(XO_VOTERS);
    for(size_t i = 0; i < XO_VOTERS; i++) {
        int voter = static_cast<int>(i + XO_VOTERS_START);
        state->voters[i].name = rpm_stat_map[voter].label;
        values = stats + (voter * MAX_RPM_PARAMS);
        state->voters[i].totalTimeInMsecVotedForSinceBoot = values[0] / RPM_CLK;
        state->voters[i].totalNumberOfTimesVotedSinceBoot = values[1];
    }

    /* Update statistics for VMIN state */
    state = &states[RPM_MODE_VMIN];
    state->name = "VMIN";
    values = stats + (RPM_MODE_VMIN * MAX_RPM_PARAMS);

    state->residencyInMsecSinceBoot = values[1];
    state->totalTransitions = values[0];
    state->supportedOnlyInSuspend = false;
    state->voters.resize(VMIN_VOTERS);
    //Note: No filling of state voters since VMIN_VOTERS = 0

done:
    _hidl_cb(states, Status::SUCCESS);
    return Void();
}

static int get_wlan_low_power_stats(struct PowerStateSubsystem *subsystem) {
    uint64_t stats[WLAN_POWER_PARAMS_COUNT] = {0};
    struct PowerStateSubsystemSleepState *state;

    subsystem->name = "wlan";

    if (extract_wlan_stats(stats) != 0) {
        subsystem->states.resize(0);
        return -1;
    }

    subsystem->states.resize(WLAN_STATES_COUNT);

    /* Update statistics for Active State */
    state = &subsystem->states[WLAN_STATE_ACTIVE];
    state->name = "Active";
    state->residencyInMsecSinceBoot = stats[CUMULATIVE_TOTAL_ON_TIME_MS];
    state->totalTransitions = stats[DEEP_SLEEP_ENTER_COUNTER];
    state->lastEntryTimestampMs = 0; //FIXME need a new value from Qcom
    state->supportedOnlyInSuspend = false;

    /* Update statistics for Deep-Sleep state */
    state = &subsystem->states[WLAN_STATE_DEEP_SLEEP];
    state->name = "Deep-Sleep";
    state->residencyInMsecSinceBoot = stats[CUMULATIVE_SLEEP_TIME_MS];
    state->totalTransitions = stats[DEEP_SLEEP_ENTER_COUNTER];
    state->lastEntryTimestampMs = stats[LAST_DEEP_SLEEP_ENTER_TSTAMP_MS];
    state->supportedOnlyInSuspend = false;

    return 0;
}

enum easel_state {
    EASEL_OFF = 0,
    EASEL_ON,
    EASEL_SUSPENDED,
    NUM_EASEL_STATES
};

// Get low power stats for easel subsystem
static int get_easel_low_power_stats(struct PowerStateSubsystem *subsystem) {
    // This implementation is a workaround to provide minimal visibility into
    // Easel state behavior until canonical low power stats are supported.
    // It takes an "external observer" snapshot of the current Easel state every
    // time it is called, and synthesizes an artificial sleep state that will
    // behave similarly to real stats if Easel gets "wedged" in the "on" state.
    static std::mutex statsLock;
    static uint64_t totalOnSnapshotCount = 0;
    static uint64_t totalNotOnSnapshotCount = 0;
    unsigned long currentState;
    struct PowerStateSubsystemSleepState *state;

    subsystem->name = "Easel";

    if (get_easel_state(&currentState) != 0) {
        subsystem->states.resize(0);
        return -1;
    }

    if (currentState >= NUM_EASEL_STATES) {
        ALOGE("%s: unrecognized Easel state(%lu)", __func__, currentState);
        return -1;
    }

    subsystem->states.resize(1);

    // Since we are storing stats locally but can have multiple parallel
    // callers, locking is required to ensure stats are not corrupted.
    std::lock_guard<std::mutex> lk(statsLock);

    // Update statistics for synthetic sleep state.  We combine OFF and
    // SUSPENDED to act as a composite "not on" state so the numbers will behave
    // like a real sleep state.
    if ((currentState == EASEL_OFF) || (currentState == EASEL_SUSPENDED)) {
        totalNotOnSnapshotCount++;
    } else {
        totalOnSnapshotCount++;
    }

    // Update statistics for synthetic sleep state, where
    // totalTransitions = cumulative count of Easel state0 (as seen by PowerHAL)
    // residencyInMsecsSinceBoot = cumulative count of Easel state1 (as seen by
    //   PowerHAL)
    // lastEntryTimestampMs = cumulative count of Easel state2 (as seen by
    //   PowerHAL)
    state = &subsystem->states[0];
    state->name = "SyntheticSleep";
    state->totalTransitions = totalOnSnapshotCount;
    state->residencyInMsecSinceBoot = totalNotOnSnapshotCount;
    state->lastEntryTimestampMs = 0;  // No added value for the workaround
    state->supportedOnlyInSuspend = false;

    return 0;
}

// Methods from ::android::hardware::power::V1_1::IPower follow.
Return<void> Power::getSubsystemLowPowerStats(getSubsystemLowPowerStats_cb _hidl_cb) {
    hidl_vec<PowerStateSubsystem> subsystems;

    subsystems.resize(SUBSYSTEM_COUNT);

    // Get WLAN subsystem low power stats.
    if (get_wlan_low_power_stats(&subsystems[SUBSYSTEM_WLAN]) != 0) {
        ALOGE("%s: failed to process wlan stats", __func__);
    }

    // Get Easel subsystem low power stats.
    if (get_easel_low_power_stats(&subsystems[SUBSYSTEM_EASEL]) != 0) {
        ALOGE("%s: failed to process Easel stats", __func__);
    }

    _hidl_cb(subsystems, Status::SUCCESS);
    return Void();
}

bool Power::isSupportedGovernor() {
    std::string buf;
    if (android::base::ReadFileToString("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", &buf)) {
        buf = android::base::Trim(buf);
    }
    // Only support EAS 1.2, legacy EAS
    if (buf == "schedutil" || buf == "sched") {
        return true;
    } else {
        LOG(ERROR) << "Governor not supported by powerHAL, skipping";
        return false;
    }
}

Return<void> Power::powerHintAsync(PowerHint_1_0 hint, int32_t data) {
    // just call the normal power hint in this oneway function
    return powerHint(hint, data);
}

// Methods from ::android::hardware::power::V1_2::IPower follow.
Return<void> Power::powerHintAsync_1_2(PowerHint_1_2 hint, int32_t data) {
    if (!isSupportedGovernor()) {
        return Void();
    }

    switch(hint) {
        case PowerHint_1_2::AUDIO_LOW_LATENCY:
            ATRACE_BEGIN("audio_low_latency");
            if (data) {
                // Hint until canceled
                ATRACE_INT("audio_low_latency_lock", 1);
                mHintManager->DoHint("AUDIO_LOW_LATENCY");
                ALOGD("AUDIO LOW LATENCY ON");
                if (!android::base::SetProperty(kPowerHalAudioProp, "LOW_LATENCY")) {
                    ALOGE("%s: could not set powerHAL audio state property to LOW_LATENCY", __func__);
                }
            } else {
                ATRACE_INT("audio_low_latency_lock", 0);
                mHintManager->EndHint("AUDIO_LOW_LATENCY");
                ALOGD("AUDIO LOW LATENCY OFF");
                if (!android::base::SetProperty(kPowerHalAudioProp, "")) {
                    ALOGE("%s: could not clear powerHAL audio state property", __func__);
                }
            }
            ATRACE_END();
            break;
        case PowerHint_1_2::AUDIO_STREAMING:
            ATRACE_BEGIN("audio_streaming");
            if (data) {
                // Hint until canceled
                ATRACE_INT("audio_streaming_lock", 1);
                mHintManager->DoHint("AUDIO_STREAMING");
                ALOGD("AUDIO STREAMING ON");
            } else {
                ATRACE_INT("audio_streaming_lock", 0);
                mHintManager->EndHint("AUDIO_STREAMING");
                ALOGD("AUDIO STREAMING OFF");
            }
            ATRACE_END();
            break;
        case PowerHint_1_2::CAMERA_LAUNCH:
            ATRACE_BEGIN("camera_launch");
            if (data > 0) {
                ATRACE_INT("camera_launch_lock", 1);
                mHintManager->DoHint("CAMERA_LAUNCH", std::chrono::milliseconds(data));
                ALOGD("CAMERA LAUNCH ON: %d MS, LAUNCH ON: 2500 MS", data);
                // boosts 2.5s for launching
                mHintManager->DoHint("LAUNCH", std::chrono::milliseconds(2500));
            } else if (data == 0) {
                ATRACE_INT("camera_launch_lock", 0);
                mHintManager->EndHint("CAMERA_LAUNCH");
                ALOGD("CAMERA LAUNCH OFF");
            } else {
                ALOGE("CAMERA LAUNCH INVALID DATA: %d", data);
            }
            ATRACE_END();
            break;
        case PowerHint_1_2::CAMERA_STREAMING:
            ATRACE_BEGIN("camera_streaming");
            if (data > 0) {
                ATRACE_INT("camera_streaming_lock", 1);
                mHintManager->DoHint("CAMERA_STREAMING", std::chrono::milliseconds(data));
                ALOGD("CAMERA STREAMING ON: %d MS", data);
            } else if (data == 0) {
                ATRACE_INT("camera_streaming_lock", 0);
                mHintManager->EndHint("CAMERA_STREAMING");
                ALOGD("CAMERA STREAMING OFF");
            } else {
                ALOGE("CAMERA STREAMING INVALID DATA: %d", data);
            }
            ATRACE_END();
            break;
        case PowerHint_1_2::CAMERA_SHOT:
            ATRACE_BEGIN("camera_shot");
            if (data > 0) {
                ATRACE_INT("camera_shot_lock", 1);
                mHintManager->DoHint("CAMERA_SHOT", std::chrono::milliseconds(data));
                ALOGD("CAMERA SHOT ON: %d MS", data);
            } else if (data == 0) {
                ATRACE_INT("camera_shot_lock", 0);
                mHintManager->EndHint("CAMERA_SHOT");
                ALOGD("CAMERA SHOT OFF");
            } else {
                ALOGE("CAMERA SHOT INVALID DATA: %d", data);
            }
            ATRACE_END();
            break;
        default:
            return powerHint(static_cast<PowerHint_1_0>(hint), data);
    }
    return Void();
}

constexpr const char* boolToString(bool b) {
    return b ? "true" : "false";
}

Return<void> Power::debug(const hidl_handle& handle, const hidl_vec<hidl_string>&) {
    if (handle != nullptr && handle->numFds >= 1) {
        int fd = handle->data[0];

        std::string buf(android::base::StringPrintf("HintManager Running: %s\n"
                                                    "VRMode: %s\n"
                                                    "SustainedPerformanceMode: %s\n"
                                                    "VideoEncodeMode: %s\n",
                                                    boolToString(mHintManager->IsRunning()),
                                                    boolToString(mVRModeOn),
                                                    boolToString(mSustainedPerfModeOn),
                                                    boolToString(mEncoderModeOn)));
        // Dump nodes through libperfmgr
        mHintManager->DumpToFd(fd);
        if (!android::base::WriteStringToFd(buf, fd)) {
            PLOG(ERROR) << "Failed to dump state to fd";
        }
        fsync(fd);
    }
    return Void();
}

}  // namespace implementation
}  // namespace V1_2
}  // namespace power
}  // namespace hardware
}  // namespace android
