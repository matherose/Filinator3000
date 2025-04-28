# Version information for release tagging
MAJOR = 0
MINOR = 1
PATCH = 0

CC = gcc
TARGET = filinator
SRC_DIR = src
BUILD_DIR = build

# Source files
SRCS = $(SRC_DIR)/filinator.c \
       $(SRC_DIR)/file_ops.c \
       $(SRC_DIR)/path_transform.c \
       $(SRC_DIR)/platform_unix.c \
       $(SRC_DIR)/platform_win.c

# Object files
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Include directories
INCLUDES = -I$(SRC_DIR)

# Determine platform-specific flags
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    PLATFORM_FLAGS = -DPLATFORM_UNIX=1
else ifeq ($(UNAME_S),Linux)
    PLATFORM_FLAGS = -DPLATFORM_UNIX=1
else ifeq ($(OS),Windows_NT)
    PLATFORM_FLAGS = -DPLATFORM_WIN32=1
else
    # Default to Unix-like systems if detection fails
    PLATFORM_FLAGS = -DPLATFORM_UNIX=1
endif

# C89 flags with POSIX compatibility for maximum portability
# -Wall, -Wextra and -pedantic ensure code quality and standard compliance
CFLAGS = -std=c89 -O2 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -pedantic $(PLATFORM_FLAGS) $(INCLUDES)

# Default target - builds the executable
all: $(BUILD_DIR) $(TARGET)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Removes compiled binary and build directory
clean:
	rm -f $(TARGET)
	rm -rf $(BUILD_DIR)

.PHONY: all clean release

# Creates a new version release with git tag
# Requires git repository to be properly configured
release: all clean
	@echo "Version: $(MAJOR).$(MINOR).$(PATCH)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "CC: $(CC)"
	@echo "TARGET: $(TARGET)"
	@echo "SRC_DIR: $(SRC_DIR)"

	# Check if there are changes to commit
	@if [ -n "$$(git status --porcelain)" ]; then \
		echo "Changes detected, creating commit and tag..."; \
		git add .; \
		git commit -m "Version $(MAJOR).$(MINOR).$(PATCH)"; \
		git push origin main; \
	else \
		echo "No changes detected, creating tag only..."; \
	fi

	# Create tag and push it
	git tag -a v$(MAJOR).$(MINOR).$(PATCH) -m "Version $(MAJOR).$(MINOR).$(PATCH)" || true
	git push origin v$(MAJOR).$(MINOR).$(PATCH) || true
