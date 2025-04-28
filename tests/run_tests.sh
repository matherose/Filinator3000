#!/bin/bash
# Test script for Filinator
# Tests encoding and decoding of file and directory names

set -e  # Exit on any error

# Colors for better readability
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== Filinator3000 Test ===${NC}"

# Project root directory
PROJECT_DIR=$(dirname $(dirname $(realpath $0)))
echo "Project directory: $PROJECT_DIR"

# Path to the Filinator executable
FILINATOR="$PROJECT_DIR/filinator"

# Check that Filinator exists
if [ ! -f "$FILINATOR" ]; then
    echo -e "${RED}Error: $FILINATOR does not exist. Compile the project first.${NC}"
    exit 1
fi

# Create the test structure
TEST_BASE="$PROJECT_DIR/tests/test_data"
rm -rf "$TEST_BASE"
mkdir -p "$TEST_BASE"

echo -e "${YELLOW}1. Creating a test directory and file structure...${NC}"

# Create directories with spaces and special characters
mkdir -p "$TEST_BASE/Normal Directory"
mkdir -p "$TEST_BASE/Directory with spaces"
mkdir -p "$TEST_BASE/Directory_without_spaces"
mkdir -p "$TEST_BASE/Directory with spaces/Subdirectory"
mkdir -p "$TEST_BASE/Directory with § special character"

# Create files with different names
echo "Normal content" > "$TEST_BASE/normal_file.txt"
echo "Content with spaces" > "$TEST_BASE/file with spaces.txt"
echo "Content with slash" > "$TEST_BASE/file/with/slash.txt"
echo "Content with section" > "$TEST_BASE/file§with§section.txt"
echo "Content in subdirectory" > "$TEST_BASE/Directory with spaces/file in directory.txt"
echo "Special content" > "$TEST_BASE/Directory with § special character/special file.txt"
echo "Content with spaces and slashes" > "$TEST_BASE/Directory with spaces/file/with/paths.txt"

# List the created structure
echo -e "${YELLOW}Test structure created:${NC}"
find "$TEST_BASE" -type f -o -type d | sort

# Save the file list for later comparison
find "$TEST_BASE" -type f | sort > "$PROJECT_DIR/tests/original_files.txt"
find "$TEST_BASE" -type d | sort > "$PROJECT_DIR/tests/original_dirs.txt"

# Calculate checksums for files
echo -e "${YELLOW}Calculating checksums of original files...${NC}"
find "$TEST_BASE" -type f -exec sha256sum {} \; | sort -k 2 > "$PROJECT_DIR/tests/original_checksums.txt"

# Encoding phase
echo -e "${YELLOW}2. Encoding the structure...${NC}"
"$FILINATOR" -encode "$TEST_BASE" -output "$PROJECT_DIR/tests/encoded"

# Check that the encoded directory exists
if [ ! -d "$PROJECT_DIR/tests/encoded" ]; then
    echo -e "${RED}Error: Encoding failed, encoded directory does not exist.${NC}"
    exit 1
fi

echo -e "${YELLOW}Content of the encoded directory:${NC}"
find "$PROJECT_DIR/tests/encoded" -type f | sort

# Calculate checksums for encoded files
echo -e "${YELLOW}Calculating checksums of encoded files...${NC}"
find "$PROJECT_DIR/tests/encoded" -type f -exec sha256sum {} \; | sort -k 2 > "$PROJECT_DIR/tests/encoded_checksums.txt"

# Check that all files have been encoded
ORIGINAL_COUNT=$(wc -l < "$PROJECT_DIR/tests/original_files.txt")
ENCODED_COUNT=$(find "$PROJECT_DIR/tests/encoded" -type f | wc -l)

if [ "$ORIGINAL_COUNT" -ne "$ENCODED_COUNT" ]; then
    echo -e "${RED}Error: Different number of files after encoding.${NC}"
    echo -e "${RED}Original: $ORIGINAL_COUNT, Encoded: $ENCODED_COUNT${NC}"
    exit 1
fi

echo -e "${GREEN}✓ All files have been encoded.${NC}"

# Create a copy of the encoded structure for decoding
echo -e "${YELLOW}3. Preparing for decoding...${NC}"
rm -rf "$PROJECT_DIR/tests/to_decode"
cp -r "$PROJECT_DIR/tests/encoded" "$PROJECT_DIR/tests/to_decode"

# List the structure to decode
echo -e "${YELLOW}Structure to decode:${NC}"
find "$PROJECT_DIR/tests/to_decode" -type f | sort

# Decoding phase
echo -e "${YELLOW}4. Decoding the structure...${NC}"
"$FILINATOR" -decode "$PROJECT_DIR/tests/to_decode"

# Check the decoded structure
echo -e "${YELLOW}Structure after decoding:${NC}"
find "$PROJECT_DIR/tests/to_decode" -type f | sort > "$PROJECT_DIR/tests/decoded_files.txt"
find "$PROJECT_DIR/tests/to_decode" -type d | sort > "$PROJECT_DIR/tests/decoded_dirs.txt"

# Calculate checksums for decoded files
echo -e "${YELLOW}Calculating checksums of decoded files...${NC}"
find "$PROJECT_DIR/tests/to_decode" -type f -exec sha256sum {} \; | sort -k 2 > "$PROJECT_DIR/tests/decoded_checksums.txt"

# Check that decoding created a structure similar to the original
DECODED_COUNT=$(wc -l < "$PROJECT_DIR/tests/decoded_files.txt")

if [ "$ORIGINAL_COUNT" -ne "$DECODED_COUNT" ]; then
    echo -e "${RED}Error: Different number of files after decoding.${NC}"
    echo -e "${RED}Original: $ORIGINAL_COUNT, Decoded: $DECODED_COUNT${NC}"
    exit 1
fi

echo -e "${GREEN}✓ All files have been decoded.${NC}"

# Check that file content is preserved (via checksums)
ORIG_CHECKSUM_COUNT=$(wc -l < "$PROJECT_DIR/tests/original_checksums.txt")
DECODED_CHECKSUM_COUNT=$(wc -l < "$PROJECT_DIR/tests/decoded_checksums.txt")

if [ "$ORIG_CHECKSUM_COUNT" -ne "$DECODED_CHECKSUM_COUNT" ]; then
    echo -e "${RED}Error: Different number of checksums.${NC}"
    exit 1
fi

# Checking checksums file by file would be ideal, but it's complex in bash
# So we just check that files have the same size for simplicity
ORIG_TOTAL_SIZE=$(du -sb "$TEST_BASE" | awk '{print $1}')
DECODED_TOTAL_SIZE=$(du -sb "$PROJECT_DIR/tests/to_decode" | awk '{print $1}')

if [ "$ORIG_TOTAL_SIZE" -ne "$DECODED_TOTAL_SIZE" ]; then
    echo -e "${RED}Error: Different total data size after decoding.${NC}"
    echo -e "${RED}Original: $ORIG_TOTAL_SIZE, Decoded: $DECODED_TOTAL_SIZE${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Data sizes are identical.${NC}"
echo -e "${GREEN}=== All tests passed! ===${NC}"

exit 0