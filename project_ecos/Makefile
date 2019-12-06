#Esta makefile faz tudo automáticamente, procura os sources e ainda faz a gestão das dependências. 
#Basta meter todos os .c e .h numa pasta chamada "src", a MakeFile estará na parent directory do src.
#Francisco Melo

INSTALL_DIR=/home/sce/ecos-work/pc_install
include $(INSTALL_DIR)/include/pkgconf/ecos.mak

TARGET_EXEC ?=program
LDFLAGS = -nostartfiles -L$(INSTALL_DIR)/lib -Ttarget.ld $(ECOS_GLOBAL_LDFLAGS)
CFLAGS = -I$(INSTALL_DIR)/include $(ECOS_GLOBAL_CFLAGS)
CC = $(ECOS_COMMAND_PREFIX)gcc

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.c)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

FLAGS ?= $(INC_FLAGS) $(CFLAGS)

$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(FLAGS) $(FLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR) $(TARGET_EXEC)

-include $(DEPS)

MKDIR_P ?= mkdir -p
