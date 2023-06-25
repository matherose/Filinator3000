#!/bin/bash

# Script to encode and decode file and folder names

SCRIPT_NAME="$0"

# Encode folders by replacing spaces with "§"
encode_folders() {
  find . -depth -name "* *" -execdir bash -c 'mv "$1" "${1// /§}"' _ {} \;
}

# Encode files by replacing characters in the file path
encode_files() {
  while IFS= read -r -d '' file; do
    # Skip encoding the script file
    if [[ "$file" != "$SCRIPT_NAME" ]]; then
      # Encode the file path by replacing characters with their encoded counterparts
      fullpath=$(readlink -f "$file" | sed -r "s/\//@/g" | sed -r "s/ /_/g" | sed -r "s/§/ /g" | sed -r "s/^\.\///")
      mv "$file" "$fullpath"  # Rename the file with the encoded file path
    fi
  done < <(find . -type f -print0)
}

# Find and remove empty folders
find_empty_folders() {
  find . -depth -type d -empty -exec rmdir {} \;
}

# Decode files by restoring characters in the file path
decode_files() {
  while IFS= read -r -d '' file; do
    # Skip decoding the script file
    if [[ "$file" != "$SCRIPT_NAME" ]]; then
      # Decode the file path by replacing encoded characters with their original counterparts
      filename=$(echo "$file" | sed -r "s/@/\//g" | sed -r "s/^\.\///" | sed -r "s/§/ /g" | sed -r "s/_/ /g" | sed -r "s/^\///")
      mkdir -p "$(dirname "$filename")"  # Create the directory structure for the decoded file path
      mv "$file" "$filename"  # Rename the file with the decoded file path
    fi
  done < <(find . -type f -print0)
}

# Decode folders by replacing "§" with spaces
decode_folders() {
  find . -depth -name "*§*" -execdir bash -c 'mv "$1" "${1//§/ }"' _ {} \;
}

# Main function
main() {
  case $1 in
    --encode)
      # Encode operation: encode folders, encode files, and remove empty folders
      encode_folders
      encode_files
      find_empty_folders
      ;;
    --decode)
      # Decode operation: decode files, decode folders, and remove empty folders
      decode_files
      decode_folders
      find_empty_folders
      ;;
    *)
      echo "Invalid argument. Usage: $0 [--encode|--decode]"
      exit 1
      ;;
  esac
}

# Check the number of command-line arguments
if [[ $# -ne 1 ]]; then
  echo "Invalid number of arguments. Usage: $0 [--encode|--decode]"
  exit 1
fi

# Call the main function with the provided argument
main "$1"
