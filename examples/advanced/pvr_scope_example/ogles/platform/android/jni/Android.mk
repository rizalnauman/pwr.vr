LOCAL_PATH := $(call my-dir)/../../..
PVRSDKDIR := $(realpath $(call my-dir)/../../../../../../..)

ASSETDIR := $(PVRSDKDIR)/examples/advanced/pvr_scope_example/ogles/platform/android/assets


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
# Prebuilt module PVRScopeDeveloper
include $(CLEAR_VARS)
LOCAL_MODULE := PVRScopeDeveloper
LOCAL_SRC_FILES := ../../../../builds/android/$(TARGET_ARCH_ABI)/lib/libPVRScopeDeveloper.a
include $(PREBUILT_STATIC_LIBRARY)
endif


# Module ogles_pvr_scope_example
include $(CLEAR_VARS)

LOCAL_MODULE    := ogles_pvr_scope_example

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := ogles_pvr_scope_example.cpp \
                    ../pvr_scope_graph.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. \
                    $(PVRSDKDIR)/framework \
                    $(PVRSDKDIR)/builds/include



LOCAL_LDLIBS := -ldl \
                -latomic \
                -llog \
                -landroid

LOCAL_STATIC_LIBRARIES := PVREngineUtils PVRGles PVRNativeGles PVRAssets PVRCore PVRScopeDeveloper android_native_app_glue


LOCAL_CFLAGS += $(SDK_BUILD_FLAGS)

LOCAL_WHOLE_STATIC_LIBRARIES := PVRShell

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
