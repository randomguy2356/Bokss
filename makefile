PROJECT := Bokss

# ---- Optional suffixes ----
WIN_SUFFIX   ?=
LINUX_SUFFIX ?=

# ---- Sources ----
SRC := $(wildcard src/*.c)
# For nested sources:
# SRC := $(shell find src -name '*.c')

# ---- Compilers ----
CC_LINUX   ?= gcc
CC_WINDOWS ?= x86_64-w64-mingw32-gcc   # requires mingw-w64

# ---- Flags ----
CFLAGS_COMMON := -Iinclude -Wall -Wextra -std=c23
CFLAGS_LINUX  ?= -O0 -g
CFLAGS_WIN    ?= -O2 -g -DWIN32 -D_WINDOWS -Os -ffunction-sections -fdata-sections
LDFLAGS_WIN   ?= -Wl,-subsystem,windows -Wl,-e,mainCRTStartup -Wl,--gc-sections
LDLIBS_LINUX := -lGL -lglfw -lm
LDLIBS_WIN   := -lopengl32 -lglfw3 -lgdi32

# ---- Build dirs / objects ----
OBJDIR_LINUX   := build/linux
OBJDIR_WINDOWS := build/windows

OBJ_LINUX   := $(SRC:src/%.c=$(OBJDIR_LINUX)/%.o)
OBJ_WINDOWS := $(SRC:src/%.c=$(OBJDIR_WINDOWS)/%.o)

# ---- Phony ----
.PHONY: all linux windows run clean

all: linux windows

# ---- Linux build ----
linux: $(OBJDIR_LINUX)/$(PROJECT) $(OBJDIR_LINUX)/$(PROJECT)$(LINUX_SUFFIX).zip

$(OBJDIR_LINUX)/$(PROJECT): $(OBJ_LINUX)
	$(CC_LINUX) $^ -o $@ $(LDLIBS_LINUX)

$(OBJDIR_LINUX)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC_LINUX) $(CFLAGS_COMMON) $(CFLAGS_LINUX) -MMD -MP -c $< -o $@

$(OBJDIR_LINUX)/$(PROJECT)$(LINUX_SUFFIX).zip: $(OBJDIR_LINUX)/$(PROJECT)
	@echo "Packaging Linux build..."
	@cd $(OBJDIR_LINUX) && \
	  PKG_DIR="$(PROJECT)$(LINUX_SUFFIX)_linux" && \
	  rm -rf "$$PKG_DIR" && mkdir -p "$$PKG_DIR" && \
	  cp "$(PROJECT)" "$$PKG_DIR"/ && cp -r ../../shaders "$$PKG_DIR"/ && \
	  zip -r "$$PKG_DIR.zip" "$$PKG_DIR" && mv "$$PKG_DIR.zip" "$(PROJECT)$(LINUX_SUFFIX).zip" && \
	  rm -rf "$$PKG_DIR"

# ---- Windows build ----
windows: $(OBJDIR_WINDOWS)/$(PROJECT).exe $(OBJDIR_WINDOWS)/$(PROJECT)$(WIN_SUFFIX).zip

$(OBJDIR_WINDOWS)/$(PROJECT).exe: $(OBJ_WINDOWS)
	$(CC_WINDOWS) $^ -o $@ $(LDFLAGS_WIN) $(LDLIBS_WIN)
	@x86_64-w64-mingw32-strip --strip-unneeded $@ 2>/dev/null || true

$(OBJDIR_WINDOWS)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC_WINDOWS) $(CFLAGS_COMMON) $(CFLAGS_WIN) -MMD -MP -c $< -o $@

$(OBJDIR_WINDOWS)/$(PROJECT)$(WIN_SUFFIX).zip: $(OBJDIR_WINDOWS)/$(PROJECT).exe
	@echo "Packaging Windows build..."
	@cd $(OBJDIR_WINDOWS) && \
	  PKG_DIR="$(PROJECT)$(WIN_SUFFIX)_windows" && \
	  rm -rf "$$PKG_DIR" && mkdir -p "$$PKG_DIR" && \
	  cp "$(PROJECT).exe" "$$PKG_DIR"/ && cp -r ../../shaders "$$PKG_DIR"/ && \
	  zip -r "$$PKG_DIR.zip" "$$PKG_DIR" && mv "$$PKG_DIR.zip" "$(PROJECT)$(WIN_SUFFIX).zip" && \
	  rm -rf "$$PKG_DIR"

# ---- Run (Linux) ----
run: linux
	./$(OBJDIR_LINUX)/$(PROJECT)

# ---- Clean ----
clean:
	rm -rf build

# ---- Dependencies ----
-include $(OBJ_LINUX:.o=.d)
-include $(OBJ_WINDOWS:.o=.d)
