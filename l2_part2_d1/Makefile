# TODO (0): Name your app
APP := ???_app
# TODO (1): Add your kernels
KERNELS := ???

.PHONY: help

help::
	$(ECHO) "Makefile Usage:"
	$(ECHO) "  make all TARGET=<sw_emu/hw_emu/hw> DEVICE=<FPGA platform> HOST_ARCH=<aarch32/aarch64/x86> SYSROOT=<sysroot_path>"
	$(ECHO) "      Command to generate the design for specified Target and Shell."
	$(ECHO) "      By default, HOST_ARCH=x86. HOST_ARCH and SYSROOT is required for SoC shells"
	$(ECHO) ""
	$(ECHO) "  make clean "
	$(ECHO) "      Command to remove the generated non-hardware files."
	$(ECHO) ""
	$(ECHO) "  make cleanall"
	$(ECHO) "      Command to remove all the generated files."
	$(ECHO) ""
	$(ECHO) "  make sd_card TARGET=<sw_emu/hw_emu/hw> DEVICE=<FPGA platform> HOST_ARCH=<aarch32/aarch64/x86> SYSROOT=<sysroot_path>"
	$(ECHO) "      Command to prepare sd_card files."
	$(ECHO) "      By default, HOST_ARCH=x86. HOST_ARCH and SYSROOT is required for SoC shells"
	$(ECHO) ""
	$(ECHO) "  make check TARGET=<sw_emu/hw_emu/hw> DEVICE=<FPGA platform> HOST_ARCH=<aarch32/aarch64/x86> SYSROOT=<sysroot_path>"
	$(ECHO) "      Command to run application in emulation."
	$(ECHO) "      By default, HOST_ARCH=x86. HOST_ARCH and SYSROOT is required for SoC shells"
	$(ECHO) ""
	$(ECHO) "  make build TARGET=<sw_emu/hw_emu/hw> DEVICE=<FPGA platform> HOST_ARCH=<aarch32/aarch64/x86> SYSROOT=<sysroot_path>"
	$(ECHO) "      Command to build xclbin application."
	$(ECHO) "      By default, HOST_ARCH=x86. HOST_ARCH and SYSROOT is required for SoC shells"
	$(ECHO) ""
	$(ECHO) "  make cosim TARGET=hw_emu DEVICE=<FPGA platform>"
	$(ECHO) "      Command to perform HLS cosim."
	$(ECHO) ""
	$(ECHO) "  make cbin TARGET=sw_emu DEVICE=<FPGA platform>"
	$(ECHO) "      Command to build c binaires in csim"
	$(ECHO) ""

# Points to top directory of Git repository
COMMON_REPO = ../
PWD = $(shell readlink -f .)
ABS_COMMON_REPO = $(shell readlink -f $(COMMON_REPO))

TARGET := hw
HOST_ARCH := x86
SYSROOT := 

include ./utils.mk

XSA := $(call device2xsa, $(DEVICE))
TEMP_DIR := ./_x.$(TARGET).$(XSA)
CSIM_DIR := ./csim
BUILD_DIR := ./build_dir.$(TARGET).$(XSA)

VPP := v++
SDCARD := sd_card

#Include Libraries
include $(ABS_COMMON_REPO)/common/includes/opencl/opencl.mk
include $(ABS_COMMON_REPO)/common/includes/xcl2/xcl2.mk
CXXFLAGS += $(xcl2_CXXFLAGS)
LDFLAGS += $(xcl2_LDFLAGS)
HOST_SRCS += $(xcl2_SRCS)
CXXFLAGS += $(opencl_CXXFLAGS) -Wall -O0 -g -std=c++11
LDFLAGS += $(opencl_LDFLAGS)

HOST_SRCS += src/host.cpp

# Host compiler global settings
CXXFLAGS += -fmessage-length=0
LDFLAGS += -lrt -lstdc++ 

ifneq ($(HOST_ARCH), x86)
	LDFLAGS += --sysroot=$(SYSROOT)
endif

# Kernel compiler global settings
CLFLAGS += -t $(TARGET) --platform $(DEVICE) --save-temps 
ifneq ($(TARGET), hw)
	CLFLAGS += -g
endif

EXECUTABLE = host
CMD_ARGS = $(BUILD_DIR)/$(APP).xclbin
EMCONFIG_DIR = $(TEMP_DIR)
EMU_DIR = $(SDCARD)/data/emulation

BINARY_CONTAINERS += $(BUILD_DIR)/$(APP).xclbin
BINARY_CONTAINER_KERNEL_OBJS += $(KERNELS:%=$(TEMP_DIR)/%.xo)
COSIM_LOGS += $(KERNELS:%=$(TEMP_DIR)/cosim/%_cosim.log)
CSIM_BINARIES += $(KERNELS:%=$(CSIM_DIR)/%.o)

CP = cp -rf

.PHONY: all clean cleanall docs emconfig
all: check-devices $(EXECUTABLE) $(BINARY_CONTAINERS) emconfig sd_card

.PHONY: exe
exe: $(EXECUTABLE)

.PHONY: build
build: $(BINARY_CONTAINERS)

.PHONY: quick-build
quick-build: $(BINARY_CONTAINER_KERNEL_OBJS)

.PHONY: cosim
cosim: $(COSIM_LOGS)

.PHONY: cbin
cbin: $(CSIM_BINARIES)

$(COSIM_LOGS): $(TEMP_DIR)/cosim/%_cosim.log: src/%.cpp src/%_tb.cpp src/%.hpp $(TEMP_DIR)/%.xo 
	mkdir -p $(TEMP_DIR)/cosim
	cp $(TEMP_DIR)/$*/$*/$*.tcl $(TEMP_DIR)/$*/$*/$*_cosim.tcl
	sed '/add_files*/a\add_files -tb "$(PWD)/src/$*_tb.cpp"' -i $(TEMP_DIR)/$*/$*/$*_cosim.tcl
	# Replace export hw with run cosim
	sed "s/export_design/cosim_design/" -i $(TEMP_DIR)/$*/$*/$*_cosim.tcl
	# Replace the final puts
	sed "s/HLS completed successfully/cosim finished/" -i $(TEMP_DIR)/$*/$*/$*_cosim.tcl
	# Create a makefile for the Cosim and then run it -- don't pass the arguments from the current make invocation
	printf "all:\n\tvivado_hls -f $*_cosim.tcl | tee $(PWD)/$@" > $(TEMP_DIR)/$*/$*/Makefile
	make MAKEOVERRIDES= -C $(TEMP_DIR)/$*/$* all 

$(CSIM_BINARIES): $(CSIM_DIR)/%.o: src/%.cpp src/%_tb.cpp src/%.hpp
	mkdir -p $(CSIM_DIR)
	$(CXX) $(CXXFLAGS) -o $@ src/$*.cpp src/$*_tb.cpp 

# Building kernels
$(BINARY_CONTAINER_KERNEL_OBJS): $(TEMP_DIR)/%.xo: src/%.cpp
	mkdir -p $(TEMP_DIR)
	$(VPP) $(CLFLAGS) --temp_dir $(TEMP_DIR) -c -k $* -I'$(<D)' -o'$@' '$<'

$(BUILD_DIR)/$(APP).xclbin: $(BINARY_CONTAINER_KERNEL_OBJS)
	mkdir -p $(BUILD_DIR)
	$(VPP) $(CLFLAGS) --temp_dir $(BUILD_DIR) -l $(LDCLFLAGS) -o'$@' $(+)

# Building Host
$(EXECUTABLE): check-xrt $(HOST_SRCS) $(HOST_HDRS)
	$(CXX) $(CXXFLAGS) $(HOST_SRCS) $(HOST_HDRS) -o '$@' $(LDFLAGS)

emconfig:$(EMCONFIG_DIR)/emconfig.json
$(EMCONFIG_DIR)/emconfig.json:
	emconfigutil --platform $(DEVICE) --od $(EMCONFIG_DIR)

check: all
ifeq ($(TARGET),$(filter $(TARGET),sw_emu hw_emu))
ifeq ($(HOST_ARCH), x86)
	$(CP) $(EMCONFIG_DIR)/emconfig.json .
	XCL_EMULATION_MODE=$(TARGET) ./$(EXECUTABLE) $(BUILD_DIR)/$(APP).xclbin
else
	mkdir -p $(EMU_DIR)
	$(CP) $(XILINX_VITIS)/data/emulation/unified $(EMU_DIR)
	mkfatimg $(SDCARD) $(SDCARD).img 500000
	launch_emulator -no-reboot -runtime ocl -t $(TARGET) -sd-card-image $(SDCARD).img -device-family $(DEV_FAM)
endif
else
ifeq ($(HOST_ARCH), x86)
	./$(EXECUTABLE) $(BUILD_DIR)/$(APP).xclbin
endif
endif
ifeq ($(HOST_ARCH), x86)
	perf_analyze profile -i profile_summary.csv -f html
endif

sd_card: $(EXECUTABLE) $(BINARY_CONTAINERS) emconfig
ifneq ($(HOST_ARCH), x86)
	mkdir -p $(SDCARD)/$(BUILD_DIR)
	$(CP) $(B_NAME)/sw/$(XSA)/boot/generic.readme $(B_NAME)/sw/$(XSA)/xrt/image/* xrt.ini $(EXECUTABLE) $(SDCARD)
	$(CP) $(BUILD_DIR)/*.xclbin $(SDCARD)/$(BUILD_DIR)/
ifeq ($(TARGET),$(filter $(TARGET),sw_emu hw_emu))
	$(ECHO) 'cd /mnt/' >> $(SDCARD)/init.sh
	$(ECHO) 'export XILINX_VITIS=$$PWD' >> $(SDCARD)/init.sh
	$(ECHO) 'export XCL_EMULATION_MODE=$(TARGET)' >> $(SDCARD)/init.sh
	$(ECHO) './$(EXECUTABLE) $(CMD_ARGS)' >> $(SDCARD)/init.sh
	$(ECHO) 'reboot' >> $(SDCARD)/init.sh
else
	[ -f $(SDCARD)/BOOT.BIN ] && echo "INFO: BOOT.BIN already exists" || $(CP) $(BUILD_DIR)/sd_card/BOOT.BIN $(SDCARD)/
	$(ECHO) './$(EXECUTABLE) $(CMD_ARGS)' >> $(SDCARD)/init.sh
endif
endif

# Cleaning stuff
clean:
	-$(RMDIR) $(EXECUTABLE) $(XCLBIN)/{*sw_emu*,*hw_emu*} 
	-$(RMDIR) profile_* TempConfig system_estimate.xtxt *.rpt *.csv 
	-$(RMDIR) src/*.ll *v++* .Xil emconfig.json dltmp* xmltmp* *.log *.jou *.wcfg *.wdb
	-$(RMDIR) $(CSIM_DIR)

cleanall: clean
	-$(RMDIR) build_dir* sd_card*
	-$(RMDIR) _x.* *xclbin.run_summary qemu-memory-_* emulation/ _vimage/ pl* start_simulation.sh *.xclbin

