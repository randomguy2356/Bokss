PROJECT := Bokss

# ---- Optional suffixes ----
# Change or override these on the command line, e.g.:
# make WIN_SUFFIX=_v1.0 LINUX_SUFFIX=_v1.0
WIN_SUFFIX   ?=
LINUX_SUFFIX ?=

# ---- Sources ----
SRC := $(wildcard src/*.c)
# For nested sources: use this instead
# SRC := $(shell find src -name '*.c')

# ---- Compilers ----
CC_LINUX   ?= gcc
CC_WINDOWS ?= x86_64-w64-mingw32-gcc   # requires mingw-w64

# ---- Flags ----
CFLAGS_COMMON := -Iinclude -Wall -Wextra -std=c23
CFLAGS_LINUX  ?= -O0 -g
CFLAGS_WIN    ?= -O2 -g -DWIN32 -D_WINDOWS -mwindows

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
	@echo "Zipping Linux build..."
	cd $(OBJDIR_LINUX) && \
	zip -r "$(PROJECT)$(LINUX_SUFFIX)_linux.zip" "$(PROJECT)" ../../shaders/
	@mv $(OBJDIR_LINUX)/"$(PROJECT)$(LINUX_SUFFIX)_linux.zip" $@

# ---- Windows build ----
windows: $(OBJDIR_WINDOWS)/$(PROJECT).exe $(OBJDIR_WINDOWS)/$(PROJECT)$(WIN_SUFFIX).zip

$(OBJDIR_WINDOWS)/$(PROJECT).exe: $(OBJ_WINDOWS)
	$(CC_WINDOWS) $^ -o $@ $(LDLIBS_WIN)

$(OBJDIR_WINDOWS)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC_WINDOWS) $(CFLAGS_COMMON) $(CFLAGS_WIN) -MMD -MP -c $< -o $@

$(OBJDIR_WINDOWS)/$(PROJECT)$(WIN_SUFFIX).zip: $(OBJDIR_WINDOWS)/$(PROJECT).exe
	@echo "Zipping Windows build..."
	cd $(OBJDIR_WINDOWS) && \
	zip -r "$(PROJECT)$(WIN_SUFFIX)_windows.zip" "$(PROJECT).exe" ../../shaders/
	@mv $(OBJDIR_WINDOWS)/"$(PROJECT)$(WIN_SUFFIX)_windows.zip" $@

# ---- Run (Linux) ----
run: linux
	./$(OBJDIR_LINUX)/$(PROJECT)

# ---- Clean ----
clean:
	rm -rf build

# ---- Dependencies ----
-include $(OBJ_LINUX:.o=.d)
-include $(OBJ_WINDOWS:.o=.d)
