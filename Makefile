# Makefile for Filinator
# To build:         make
# To clean:         make clean
# To create release: make release
#
# You can override variables like 'TARGET' via command line, for example:
#   make TARGET=filinator.exe

CC = gcc
TARGET = filinator
SRC = filinator.c

MAJOR = 1
MINOR = 0
PATCH = 0

# C89 flags with POSIX compatibility for maximum portability
# -Wall, -Wextra and -pedantic ensure code quality and standard compliance
CFLAGS = -std=c89 -O2 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -pedantic

# Default target - builds the executable
all: $(TARGET)

# Compiles filinator from source
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Removes compiled binary and any temporary files
clean:
	rm -f $(TARGET)

.PHONY: all clean

# Creates a new version release with git tag
# Requires git repo to be properly configured
release: all clean
	@echo "Version: $(MAJOR).$(MINOR).$(PATCH)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "CC: $(CC)"
	@echo "TARGET: $(TARGET)"
	@echo "SRC: $(SRC)"

	# Create tag and push it to repository
	git add .
	git commit -m "Version $(MAJOR).$(MINOR).$(PATCH)"
	git push origin main
	git tag -a v$(MAJOR).$(MINOR).$(PATCH) -m "Version $(MAJOR).$(MINOR).$(PATCH)"
	git push origin v$(MAJOR).$(MINOR).$(PATCH)
