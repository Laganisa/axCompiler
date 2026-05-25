CROSS_COMPILE := aarch64-linux-gnu-
CC            := $(CROSS_COMPILE)gcc
LD            := $(CROSS_COMPILE)ld
OBJCOPY       := $(CROSS_COMPILE)objcopy

axLIB_DIR     ?= ../axLib
BUILD_DIR     ?= build
OBJ_DIR       := $(BUILD_DIR)/obj

TARGET_NAME   ?= COMPILER
TARGET_ELF    := $(BUILD_DIR)/$(TARGET_NAME).elf
TARGET_IMAGE  := $(BUILD_DIR)/$(TARGET_NAME).bin
TARGET_BIN    := $(TARGET_NAME).BIN

SRC_DIRS      := asm compiler
SRC_C         := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
SRC_S         := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.S))
OBJS          := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC_C)) \
                 $(patsubst %.S,$(OBJ_DIR)/%.o,$(SRC_S))

CPPFLAGS      += -Iinclude -I$(axLIB_DIR)/include
CFLAGS        += -mcpu=cortex-a72 -ffreestanding -fno-builtin -nostdlib -Wall -Wextra -O2
LDFLAGS       += -T linker.ld
LDLIBS        += $(axLIB_DIR)/build/libaxlib.a

FM_EXEC_MAGIC ?= 0x4D594F535441534B
FM_EXEC_MODE  ?= 1

MKDIR_P       ?= mkdir -p
RM_RF         ?= rm -rf

.PHONY: all clean axlib

all: $(TARGET_BIN)

axlib:
	@$(MAKE) -C $(axLIB_DIR) --no-print-directory

$(OBJ_DIR):
	@$(MKDIR_P) $@

$(BUILD_DIR):
	@$(MKDIR_P) $@

$(OBJ_DIR)/asm $(OBJ_DIR)/compiler:
	@$(MKDIR_P) $@

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR) $(OBJ_DIR)/asm $(OBJ_DIR)/compiler
	@echo "CC  $<"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.S | $(OBJ_DIR) $(OBJ_DIR)/asm $(OBJ_DIR)/compiler
	@echo "AS  $<"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TARGET_ELF): axlib $(OBJS) | $(BUILD_DIR)
	@echo "LD  $@"
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(TARGET_IMAGE): $(TARGET_ELF)
	@echo "OBJCOPY  $@"
	$(OBJCOPY) $< -O binary $@

$(TARGET_BIN): $(TARGET_IMAGE)
	@echo "PACK  $@"
	@python3 -c "from pathlib import Path; import struct; image = Path(r'$(TARGET_IMAGE)').read_bytes(); hdr = struct.pack('<QQQQ', int('$(FM_EXEC_MAGIC)', 0), int('$(FM_EXEC_MODE)', 0), 0, len(image)); Path(r'$(TARGET_BIN)').write_bytes(hdr + image)"

clean:
	$(RM_RF) $(BUILD_DIR) $(TARGET_BIN)
