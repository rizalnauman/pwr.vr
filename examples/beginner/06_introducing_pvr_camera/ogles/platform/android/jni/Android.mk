LOCAL_PATH := $(call my-dir)/../../..
PVRSDKDIR := $(realpath $(call my-dir)/../../../../../../..)

ASSETDIR := $(PVRSDKDIR)/examples/beginner/06_introducing_pvr_camera/ogles/platform/android/assets
LIBDIR := $(PVRSDKDIR)/examples/beginner/06_introducing_pvr_camera/ogles/platform/android/libs

ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVREngineUtils
include $(CLEAR_VARS)
LOCAL_MODULE := PVREngineUtils
LOCAL_SRC_FILES := $(PVRSDKDIR)/framework/Bin/Android/local/$(TARGET_ARCH_ABI)/libPVREngineUtils.a
include $(PREBUILT_STATIC_LIBRARY)
endif

ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVRShell
include $(CLEAR_VARS)
LOCAL_MODULE := PVRShell
LOCAL_SRC_FILES := $(PVRSDKDIR)/framework/Bin/Android/local/$(TARGET_ARCH_ABI)/libPVRShell.a
include $(PREBUILT_STATIC_LIBRARY)
endif

ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVRGles
include $(CLEAR_VARS)
LOCAL_MODULE := PVRGles
LOCAL_SRC_FILES := $(PVRSDKDIR)/framework/Bin/Android/local/$(TARGET_ARCH_ABI)/libPVRGles.a
include $(PREBUILT_STATIC_LIBRARY)
endif

ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVRNativeGles
include $(CLEAR_VARS)
LOCAL_MODULE := PVRNativeGles
LOCAL_SRC_FILES := $(PVRSDKDIR)/framework/Bin/Android/local/$(TARGET_ARCH_ABI)/libPVRNativeGles.a
include $(PREBUILT_STATIC_LIBRARY)
endif

ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVRAssets
include $(CLEAR_VARS)
LOCAL_MODULE := PVRAssets
LOCAL_SRC_FILES := $(PVRSDKDIR)/framework/Bin/Android/local/$(TARGET_ARCH_ABI)/libPVRAssets.a
include $(PREBUILT_STATIC_LIBRARY)
endif

ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVRCore
include $(CLEAR_VARS)
LOCAL_MODULE := PVRCore
LOCAL_SRC_FILES := $(PVRSDKDIR)/framework/Bin/Android/local/$(TARGET_ARCH_ABI)/libPVRCore.a
include $(PREBUILT_STATIC_LIBRARY)
endif

ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVRCamera
include $(CLEAR_VARS)
LOCAL_MODULE := PVRCamera
LOCAL_SRC_FILES := $(PVRSDKDIR)/framework/Bin/Android/local/$(TARGET_ARCH_ABI)/libPVRCamera.a
include $(PREBUILT_STATIC_LIBRARY)
endif


# Module ogles_introducing_pvr_camera
include $(CLEAR_VARS)

LOCAL_MODULE    := ogles_introducing_pvr_camera

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := ogles_introducing_pvr_camera.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/framework \
                    $(PVRSDKDIR)/builds/include



LOCAL_LDLIBS := -latomic \
                -llog \
                -landroid

LOCAL_STATIC_LIBRARIES := PVREngineUtils PVRGles PVRNativeGles PVRAssets PVRCore android_native_app_glue


LOCAL_CFLAGS += $(SDK_BUILD_FLAGS)

LOCAL_WHOLE_STATIC_LIBRARIES := PVRShell PVRCamera

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
