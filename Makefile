
TARGET_ARCH ?= x86_64
DEBUG ?= 0
EXTRA_CFLAGS += -Wall -fno-strict-aliasing -Wno-undef -Wno-unused -Wno-missing-braces -Wno-missing-attributes -Wno-overflow -Wno-missing-prototypes -Wno-missing-declarations -Werror -DZX_PCIE_BUS -DNEW_ZXFB -D__LINUX__
ifeq ($(DEBUG), 1)
	EXTRA_CFLAGS += -ggdb3 -O2 -D_DEBUG_ -DZX_TRACE_EVENT=0
else
	EXTRA_CFLAGS += -O2 -fno-strict-aliasing -DZX_TRACE_EVENT=1
endif

BIN_TYPE ?= $(shell uname -m |sed -e s/i.86/i386/)

ifneq ($(KERNELRELEASE),)
KERNEL_VERSION := $(KERNELRELEASE)
else
KERNEL_VERSION := $(shell uname -r)
endif

LINUXDIR ?= /lib/modules/$(KERNEL_VERSION)/build

DRM_VER=$(shell sed -n 's/^RHEL_DRM_VERSION = \(.*\)/\1/p' $(LINUXDIR)/Makefile)
YHQILINOS=$(shell sed -n 's/^export KBUILD_BUILD_USER=//p' $(LINUXDIR)/Makefile)

ifneq (,$(DRM_VER))
DRM_PATCH=$(shell sed -n 's/^RHEL_DRM_PATCHLEVEL = \(.*\)/\1/p' $(LINUXDIR)/Makefile)
DRM_SUBLEVEL=$(shell sed -n 's/^RHEL_DRM_SUBLEVEL = \(.*\)/\1/p' $(LINUXDIR)/Makefile)
DRM_CODE=$(shell expr $(DRM_VER) \* 65536 + 0$(DRM_PATCH) \* 256 + 0$(DRM_SUBLEVEL))
EXTRA_CFLAGS += -DDRM_VERSION_CODE=$(DRM_CODE)
endif

ifeq ($(YHQILINOS), YHKYLIN-OS)
EXTRA_CFLAGS += -DYHQILIN
KERNEL_VERSION_NUM := $(shell echo $(KERNEL_VERSION) | cut -d - -f1)
ifeq (,$(DRM_VER))
ifeq ($(KERNEL_VERSION_NUM), 4.4.131)
DRM_VER=4
DRM_PATCH=9
DRM_SUBLEVEL=0
DRM_CODE=$(shell expr $(DRM_VER) \* 65536 + 0$(DRM_PATCH) \* 256 + 0$(DRM_SUBLEVEL))
EXTRA_CFLAGS += -DDRM_VERSION_CODE=$(DRM_CODE)
endif
endif
endif

KERNEL_ZX_DIR := $(shell \
	if [ -d "/lib/modules/$(KERNEL_VERSION)/kernel/drivers/gpu" ];then\
		echo "/lib/modules/$(KERNEL_VERSION)/kernel/drivers/gpu/drm/zx";\
	else\
		echo "/lib/modules/$(KERNEL_VERSION)/kernel/drivers/char/drm";\
	fi)

ifeq ("$(M)", "")
ifeq ("$(O)", "")
ZXGPU_FULL_PATH=$(src)
else
ZXGPU_FULL_PATH=$(srctree)/$(src)
endif
else
ZXGPU_FULL_PATH=$(src)
endif

ZX_SRC_DIR_O := src

ZX_OBJ_LIST:= \
        os_interface.o  \
        zxg.o \
        zx_cec.o \
        zx_driver.o \
        zx_keymap.o \
        zx_pcie.o \
        os_shared.o \
        zx_debugfs.o \
        zx_sysfs.o \
        zx_ioctl.o \
        zx_led.o \
        zx_gem.o \
        zx_fence.o \
        zxgfx_trace_events.o \
        zx_sync.o \
        zx_atomic.o \
        zx_cbios.o \
        zx_connector.o \
        zx_crtc.o \
        zx_disp.o \
        zx_drmfb.o \
        zx_encoder.o \
        zx_irq.o \
        zx_plane.o \
        zx_fbdev.o \
        zx_backlight.o \
        zx_sink.o \
        zx_i2c.o \
        zx_audio.o

zx-objs := $(addprefix $(ZX_SRC_DIR_O)/,$(ZX_OBJ_LIST))

## for cbios
EXTRA_CFLAGS += -I${ZXGPU_FULL_PATH}/src/cbios \
             -I${ZXGPU_FULL_PATH}/src/cbios/Callback \
             -I${ZXGPU_FULL_PATH}/src/cbios/Device \
             -I${ZXGPU_FULL_PATH}/src/cbios/Device/Port \
             -I${ZXGPU_FULL_PATH}/src/cbios/Device/Monitor \
             -I${ZXGPU_FULL_PATH}/src/cbios/Device/Monitor/DSIPanel \
             -I$(ZXGPU_FULL_PATH)/src/cbios/Device/Monitor/EDPPanel \
             -I${ZXGPU_FULL_PATH}/src/cbios/Display \
             -I${ZXGPU_FULL_PATH}/src/cbios/Init \
             -I${ZXGPU_FULL_PATH}/src/cbios/Util 

ZX_CBIOS_OBJ_LIST := \
        cbios/Interface/CBios.o \
        cbios/Callback/CBiosCallbacks.o \
        cbios/Init/CBiosInit.o      \
        cbios/Util/CBiosUtil.o      \
        cbios/Util/CBiosEDID.o      \
        cbios/Display/CBiosDisplayManager.o \
        cbios/Display/CBiosPathManager.o \
        cbios/Display/CBiosMode.o      \
        cbios/Device/CBiosShare.o \
        cbios/Device/CBiosDeviceShare.o \
        cbios/Device/CBiosDevice.o    \
        cbios/Device/Port/CBiosCRT.o       \
        cbios/Device/Port/CBiosDP.o        \
        cbios/Device/Port/CBiosDSI.o       \
        cbios/Device/Port/CBiosDVO.o       \
        cbios/Device/Monitor/CBiosCRTMonitor.o \
        cbios/Device/Monitor/CBiosDPMonitor.o     \
        cbios/Device/Monitor/CBiosHDMIMonitor.o   \
        cbios/Device/Monitor/CBiosEDPPanel.o \
        cbios/Device/Monitor/EDPPanel/CBiosITN156.o \
        cbios/Device/Monitor/EDPPanel/CBiosEDO156.o \
        cbios/Device/Monitor/DSIPanel/CBiosDSIPanel.o \
        cbios/Device/Monitor/DSIPanel/CBiosHX8392A.o \
        cbios/Device/Monitor/DSIPanel/CBiosNT35595.o \
        cbios/Device/Monitor/DSIPanel/CBiosR63319.o \
        cbios/Device/Monitor/DSIPanel/CBiosR63417.o \
        cbios/Hw/CBiosChipFunc.o  \
        cbios/Hw/HwInit/CBiosInitHw.o       \
        cbios/Hw/HwCallback/CBiosCallbacksHw.o \
        cbios/Hw/HwInterface/CBiosHwInterface.o      \
        cbios/Hw/HwUtil/CBiosI2C.o       \
        cbios/Hw/HwUtil/CBiosUtilHw.o       \
        cbios/Hw/Interrupt/CBiosISR.o       \
        cbios/Hw/HwBlock/CBiosScaler.o    \
        cbios/Hw/HwBlock/CBiosIGA_Timing.o  \
        cbios/Hw/HwBlock/CBiosDIU_HDTV.o  \
        cbios/Hw/HwBlock/CBiosDIU_HDAC.o  \
        cbios/Hw/HwBlock/CBiosDIU_HDCP.o  \
        cbios/Hw/HwBlock/CBiosDIU_HDMI.o \
        cbios/Hw/HwBlock/CBiosDIU_DP.o \
        cbios/Hw/HwBlock/CBiosDIU_CRT.o \
        cbios/Hw/HwBlock/CBiosDIU_DVO.o \
        cbios/Hw/HwBlock/CBiosDIU_CSC.o \
        cbios/Hw/HwBlock/CBiosPHY_DP.o \
        cbios/Hw/Chx001/CBios_chx.o          \
        cbios/Hw/Chx001/CBiosVCP_chx.o

cbios-objs := $(addprefix $(ZX_SRC_DIR_O)/,$(ZX_CBIOS_OBJ_LIST))

ifdef Elite1K_VEPD
    cbios-objs+=Hw/CBiosPWM.o
endif

zx-objs += ${cbios-objs}

ZX_CORE_BINARY_OBJECT :=  built-in_${BIN_TYPE}.o.hex
ZX_CORE_BINARY_OBJECT_O :=  built-in_${BIN_TYPE}.o

quiet_cmd_symlink = SYMLINK $@
 cmd_symlink = ln -sf "$(realpath $<)" $@

targets += $(ZX_CORE_BINARY_OBJECT_O)


zx_core-objs := $(ZX_CORE_BINARY_OBJECT_O)

zx_core-objs += $(ZX_SRC_DIR_O)/core_module.o

ifdef CONFIG_DRM_ZXE
	obj-$(CONFIG_DRM_ZXE) := zx.o zx_core.o
else
	obj-m := zx.o zx_core.o
endif

KBUILD_CFLAGS += -I$(ZXGPU_FULL_PATH)/src

modules:
	make -C $(LINUXDIR) M=`pwd` modules

ZX_OBJ_DIR := $(ZXGPU_FULL_PATH)/objs/$(TARGET_ARCH)

$(obj)/$(ZX_CORE_BINARY_OBJECT_O): $(ZX_OBJ_DIR)/$(ZX_CORE_BINARY_OBJECT) FORCE
	$(call if_changed,symlink)
clean:
	@rm -rf .tmp_versions/
	@rm -f modules.* Module.* $(patsubst %,.%.cmd,$(zx-objs)) .core_module.o.cmd .zx_core.ko.cmd .zx_core.mod.o.cmd .zx_core.o.cmd .zx.ko.cmd .zx.mod.o.cmd .zx.o.cmd *.ko *.ur-safe .cache.mk
	@rm -f $(zx-objs) $(obj-m) core_module.o zx.mod.o zx_core.mod.o built-in_*.o zx_core.mod.c zx.mod.c .core_module.o.d .zx_driver.o.d

install:
	@if [ `whoami` != "root" ] && [ `whoami` != "ROOT" ]; then \
	echo "You must be root to install the ZhaoXin drivers."; \
	exit 1; \
	fi
	@mkdir -p $(KERNEL_ZX_DIR)
	install -c -m 0744 ./zx.ko $(KERNEL_ZX_DIR)
	install -c -m 0744 ./zx_core.ko $(KERNEL_ZX_DIR)


