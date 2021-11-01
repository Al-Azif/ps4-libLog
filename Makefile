# PRX metadata
PROJECTNAME		:= libLog

# Libraries linked into the ELF.
LIBS					:= -lSceLibcInternal -lkernel

# Directorys to include
INCLUDES      := -Iinclude

# Additional compile flags
#   ERRORFLAGS and OTHERFLAGS will be included in C and C++ flags
#   OTHERCXXFLAGS will only be included in C++ flags
ERRORFLAGS    := -Wall -Wextra -Wpedantic -Werror
OTHERFLAGS    := -O3 -std=c99 -D_DEFAULT_SOURCE
OTHERCXXFLAGS := -std=c++11

# -----------------------------------------------------------------------------
# Do not edit below this line unless you know what you are doing
# -----------------------------------------------------------------------------

TOOLCHAIN			:= $(OO_PS4_TOOLCHAIN)
ODIR					:= build
SDIR					:= src
EXTRAFLAGS    := $(INCLUDES) $(ERRORFLAGS) $(OTHERFLAGS)
CFLAGS				:= -cc1 -triple x86_64-pc-freebsd-elf -munwind-tables -fuse-init-array -emit-obj -isysroot $(TOOLCHAIN) -isystem $(TOOLCHAIN)/include $(EXTRAFLAGS)
CXXFLAGS			:= $(CFLAGS) -isystem $(TOOLCHAIN)/include/c++/v1 $(OTHERCXXFLAGS)
LFLAGS				:= -m elf_x86_64 -pie --script $(TOOLCHAIN)/link.x --eh-frame-hdr -L$(TOOLCHAIN)/lib $(LIBS) $(TOOLCHAIN)/lib/crtlib.o

CFILES				:= $(wildcard $(SDIR)/*.c)
CPPFILES			:= $(wildcard $(SDIR)/*.cpp)
ASMFILES			:= $(wildcard $(SDIR)/*.s)
OBJS					:= $(patsubst $(SDIR)/%.s, $(ODIR)/%.o, $(ASMFILES)) $(patsubst $(SDIR)/%.c, $(ODIR)/%.o, $(CFILES)) $(patsubst $(SDIR)/%.cpp, $(ODIR)/%.o, $(CPPFILES))
STUBOBJS			:= $(patsubst $(SDIR)/%.c, $(ODIR)/%.o.stub, $(CFILES)) $(patsubst $(SDIR)/%.cpp, $(ODIR)/%.o.stub, $(CPPFILES))

TARGET				= $(PROJECTNAME).prx
TARGETSTUB		= $(PROJECTNAME)_stub.so

UNAME_S				:= $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	AS					:= llvm-mc
	CC					:= clang
	CXX					:= clang++
	LD					:= ld.lld
	CDIR				:= linux
endif
ifeq ($(UNAME_S),Darwin)
	AS					:= /usr/local/opt/llvm/bin/llvm-mc
	CC					:= /usr/local/opt/llvm/bin/clang
	CXX					:= /usr/local/opt/llvm/bin/clang++
	LD					:= /usr/local/opt/llvm/bin/ld.lld
	CDIR				:= macos
endif

# Make rules
$(TARGET): $(ODIR) $(OBJS)
	$(LD) $(ODIR)/*.o -o $(ODIR)/$(PROJECTNAME).elf $(LFLAGS)
	$(TOOLCHAIN)/bin/$(CDIR)/create-lib -in=$(ODIR)/$(PROJECTNAME).elf -out=$(ODIR)/$(PROJECTNAME).oelf --paid 0x3800000000000011
	@mv $(ODIR)/$(PROJECTNAME).prx $(TARGET)
	@echo Built PRX successfully!

$(TARGETSTUB): $(ODIR) $(STUBOBJS)
	$(CXX) $(ODIR)/*.o.stub -o $(TARGETSTUB) -target x86_64-pc-linux-gnu -shared -fuse-ld=lld -ffreestanding -nostdlib -fno-builtin -L$(TOOLCHAIN)/lib $(LIBS)
	@echo Built stub successfully!

$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

$(ODIR)/%.o: $(SDIR)/%.s
	$(AS) -triple=x86_64-pc-freebsd-elf --filetype=obj -o $@ $<

$(ODIR)/%.o.stub: $(SDIR)/%.cpp
	$(CXX) -target x86_64-pc-linux-gnu -ffreestanding -nostdlib -fno-builtin -fPIC -c -isysroot $(TOOLCHAIN) -isystem $(TOOLCHAIN)/include -isystem $(TOOLCHAIN)/include/c++/v1 $(EXTRAFLAGS) $(OTHERCXXFLAGS) -o $@ $<

$(ODIR)/%.o.stub: $(SDIR)/%.c
	$(CC) -target x86_64-pc-linux-gnu -ffreestanding -nostdlib -fno-builtin -fPIC -c -isysroot $(TOOLCHAIN) -isystem $(TOOLCHAIN)/include $(EXTRAFLAGS) -o $@ $<

$(ODIR):
	@echo Starting build...
	@echo Creating build directory...
	@mkdir $@

.PHONY: all clean install
.DEFAULT_GOAL	:= all

all: $(TARGET) $(TARGETSTUB)

clean:
	@echo Cleaning up...
	@rm -rf $(TARGET) $(TARGETSTUB) $(ODIR)

install:
	@echo Installing...
	@yes | cp -f $(TARGETSTUB) $(OO_PS4_TOOLCHAIN)/lib/$(TARGETSTUB) 2>/dev/null && echo Installed!|| echo Failed to install, is it built?
