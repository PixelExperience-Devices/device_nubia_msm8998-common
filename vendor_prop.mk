#
# Copyright (C) 2018 The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#


# Audio
PRODUCT_PROPERTY_OVERRIDES += \
af.fast_track_multiplier=2\
vendor.audio_hal.period_size=192\
ro.config.vc_call_vol_steps=11\
ro.vendor.audio.sdk.fluencetype=fluence\
persist.vendor.audio.fluence.voicecall=true\
persist.vendor.audio.fluence.voicerec=false\
persist.vendor.audio.fluence.speaker=false\
vendor.audio.tunnel.encode=false\
persist.vendor.audio.ras.enabled=false\
vendor.audio.offload.buffer.size.kb=32\
audio.offload.video=true\
vendor.audio.offload.track.enable=false\
audio.deep_buffer.media=true\
vendor.voice.path.for.pcm.voip=false\
vendor.audio.offload.multiaac.enable=true\
vendor.audio.dolby.ds2.enabled=false\
vendor.audio.dolby.ds2.hardbypass=false\
vendor.audio.offload.multiple.enabled=false\
vendor.audio.offload.passthrough=false\
ro.vendor.audio.sdk.ssr=false\
vendor.audio.offload.gapless.enabled=true\
vendor.audio.safx.pbe.enabled=true\
vendor.audio.parser.ip.buffer.size=0\
vendor.audio.flac.sw.decoder.24bit=true\
persist.vendor.bt.a2dp_offload_cap=sbc-aac\
vendor.audio.use.sw.alac.decoder=true\
vendor.audio.use.sw.ape.decoder=true\
vendor.audio.hw.aac.encoder=true\
vendor.fm.a2dp.conc.disabled=true\
vendor.audio.noisy.broadcast.delay=600\
vendor.audio.offload.pstimeout.secs=3\
ro.af.client_heap_size_kbyte=7168

# Battery
PRODUCT_PROPERTY_OVERRIDES += \
ro.cutoff_voltage_mv=3200

# Bluetooth
PRODUCT_PROPERTY_OVERRIDES += \
bt.max.hfpclient.connections=1\
qcom.bluetooth.soc=cherokee\
ro.bluetooth.a4wp=false\
ro.bluetooth.emb_wp_mode=false\
ro.bluetooth.wipower=false

# Camera
PRODUCT_PROPERTY_OVERRIDES += \
camera.aux.packagelist=com.android.camera,org.codeaurora.snapcam,com.google.android.GoogleCamera\
persist.camera.HAL3.enabled=1\
persist.camera.eis.enable=1\
persist.camera.depth.focus.cb=0\
persist.camera.vdbea.switch=1\
persist.ts.rtmakeup=false\
persist.camera.CDS=Off

# CNE
PRODUCT_PROPERTY_OVERRIDES += \
persist.cne.feature=1

# Data
PRODUCT_PROPERTY_OVERRIDES += \
persist.data.mode=concurrent\
persist.data.netmgrd.qos.enable=true\
ro.use_data_netmgrd=true

# DRM
PRODUCT_PROPERTY_OVERRIDES += \
drm.service.enabled=true\

# FRP
PRODUCT_PROPERTY_OVERRIDES += \
ro.frp.pst=/dev/block/platform/soc/1da4000.ufshc/by-name/frp

# Graphics
PRODUCT_PROPERTY_OVERRIDES += \
debug.sf.enable_hwc_vds=1\
debug.sf.hw=1\
debug.sf.latch_unsignaled=1\
dev.pm.dyn_samplingrate=1\
persist.demo.hdmirotationlock=false\
ro.opengles.version=196610\
persist.debug.wfd.enable=1\
persist.sys.wfd.virtual=0\
persist.hwc.enable_vds=1\
vendor.display.disable_rotator_downscale=1 \
vendor.display.disable_rotator_split=1 \
vendor.display.disable_skip_validate=1 \
vendor.display.perf_hint_window=50 \
vendor.gralloc.enable_fb_ubwc=1

# Media
PRODUCT_PROPERTY_OVERRIDES += \
media.stagefright.enable-player=true\
media.stagefright.enable-http=true\
media.stagefright.enable-aac=true\
media.stagefright.enable-qcp=true\
media.stagefright.enable-scan=true\
mmp.enable.3g2=true\
media.aac_51_output_enabled=true\
mm.enable.smoothstreaming=true\
persist.mm.enable.prefetch=true\
vidc.enc.dcvs.extra-buff-count=2

# IZat
PRODUCT_PROPERTY_OVERRIDES += \
persist.vendor.overlay.izat.optin=rro

# NFC
PRODUCT_PROPERTY_OVERRIDES += \
persist.nfc.smartcard.config=SIM1,eSE1\
ro.nfc.port=I2C

# Perf
PRODUCT_PROPERTY_OVERRIDES += \
ro.vendor.extension_library=libqti-perfd-client.so \
ro.vendor.qti.core_ctl_max_cpu=4 \
ro.vendor.qti.core_ctl_min_cpu=0 \
ro.vendor.qti.sys.fw.bg_apps_limit=60 


# IMS
PRODUCT_PROPERTY_OVERRIDES += \
persist.dbg.volte_avail_ovr=1\
persist.dbg.vt_avail_ovr=1\
persist.vendor.qti.telephony.vt_cam_interface=1\
persist.dbg.ims_volte_enable=1\
persist.radio.videopause.mode=1\
persist.data.iwlan.enable=true

# QTI
PRODUCT_PROPERTY_OVERRIDES += \
ro.vendor.at_library=libqti-at.so\
ro.vendor.gt_library=libqti-gt.so

# RIL
PRODUCT_PROPERTY_OVERRIDES += \
rild.libpath=/vendor/lib64/libril-qc-qmi-1.so\

persist.rild.nitz_plmn=\
persist.rild.nitz_long_ons_0=\
persist.rild.nitz_long_ons_1=\
persist.rild.nitz_long_ons_2=\
persist.rild.nitz_long_ons_3=\
persist.rild.nitz_short_ons_0=\
persist.rild.nitz_short_ons_1=\
persist.rild.nitz_short_ons_2=\
persist.rild.nitz_short_ons_3=\

ril.subscription.types=NV,RUIM\
DEVICE_PROVISIONED=1\
ro.telephony.default_network=22,20\
telephony.lteOnCdmaDevice=1\
keyguard.no_require_sim=true\
persist.sys.ap.restart_level=1\
persist.sys.oem_smooth=1\
persist.sys.ssr.restart_level=3\

persist.radio.multisim.config=dsds\
persist.vendor.qcomsysd.enabled=1\
persist.radio.hw_mbn_update=0\
persist.radio.sw_mbn_update=0\
persist.radio.apm_sim_not_pwdn=1\
persist.vendor.radio.apm_sim_not_pwdn=1\
persist.vendor.radio.custom_ecc=1\
persist.vendor.radio.rat_on=combine\
persist.vendor.radio.sib16_support=1\
persist.vendor.radio.cs_srv_type=1\
persist.radio.schd.cache=3500\
persist.activate_mbn.enabled=false

# RmNet Data
PRODUCT_PROPERTY_OVERRIDES += \
persist.rmnet.data.enable=true\
persist.data.wda.enable=true\
persist.data.df.dl_mode=5\
persist.data.df.ul_mode=5\
persist.data.df.agg.dl_pkt=10\
persist.data.df.agg.dl_size=4096\
persist.data.df.mux_count=8\
persist.data.df.iwlan_mux=9\
persist.data.df.dev_name=rmnet_usb0

# Sensors
PRODUCT_PROPERTY_OVERRIDES += \
ro.vendor.sensors.dev_ori=true\
ro.vendor.sensors.pmd=true\
ro.vendor.sensors.sta_detect=true\
ro.vendor.sensors.mot_detect=true\
ro.vendor.sensors.dpc=true\
ro.vendor.sensors.multishake=true

# Shutdown
PRODUCT_PROPERTY_OVERRIDES += \
sys.vendor.shutdown.waittime=500\
ro.build.shutdown_timeout=0

# Timeservice
PRODUCT_PROPERTY_OVERRIDES += \
persist.timed.enable=true

# Wi-Fi
PRODUCT_PROPERTY_OVERRIDES += \
wifi.interface=wlan0

