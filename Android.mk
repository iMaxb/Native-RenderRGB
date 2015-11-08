LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:=       \
	SoftwareRenderer.cpp \
	Main.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
  	libui \
	libgui \
	libstagefright \
  	libstagefright_foundation \

LOCAL_C_INCLUDES := \
	$(TOP)/frameworks/native/include/media/openmax \
	$(TOP)/frameworks/av/media/libstagefright/include
	
LOCAL_STATIC_LIBRARIES := \
        libstagefright_color_conversion \

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= sfdemo

include $(BUILD_EXECUTABLE)
