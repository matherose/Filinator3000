#!/bin/bash

# Encode folders : Change spaces to §
encode_folders() {
    find . -depth -name "* *" -execdir bash -c 'mv "$1" "${1// /§}"' _ {} \;
}

# Encode files :
# - Find the full path of the file with readlink
# - Change spaces to _
# - Change / to @
# - Change the first dot to nothing
encode_files() {
    # Rename files
    find . -type f -exec sh -c '
          for file do
              # save full path without spaces in var
              fullpath=$(readlink -f $file | sed -r "s/\//@/g" | sed -r "s/ /_/g" | sed -r "s/§/ /g" | sed -r "s/^\.\///")
              # rename file
              mv "$file" "$fullpath"
          done
  ' sh {} +
}

# Remove empty folders recursively
find_empty_folders() {
    find . -depth -type d -empty -exec rmdir {} \;
}

# Decode files :
# - Change @ to /, then mkdir -p to create the folder if it doesn't exist
# - Change _ to spaces
# - Change § to spaces
decode_files() {
    find . -type f -exec sh -c '
    (
        for file do
            # restore original name without the first slash
            filename=$(echo "$file" | sed -r "s/@/\//g" | sed -r "s/^\.\///" | sed -r "s/§/ /g" | sed -r "s/_/ /g" | sed -r "s/^\///")
            # create folder if it doesnt exist without the file
            mkdir -p "$(dirname "$filename")"
            # rename file
            mv "$file" "$filename"
        done
    )
    ' sh {} +
}

# Reverse folders encoding : Change § to spaces
decode_folders() {
    find . -depth -name "*§*" -execdir bash -c 'mv "$1" "${1//§/ }"' _ {} \;
}

# Main function
main() {
    # If no argument is given, kill the script
    if [ $# -eq 0 ]; then
        echo "No argument given"
        exit 1
    fi

    # If there is more than one argument, kill the script
    if [ $# -gt 1 ]; then
        echo "Too many arguments"
        exit 1
    fi

    # If argument is not --encode or --decode, kill the script
    if [ $1 != "--encode" ] && [ $1 != "--decode" ]; then
        echo "Wrong argument"
        exit 1
    fi

    # If the argument is --encode, encode the files and folders
    if [ $1 = "--encode" ]; then
        encode_folders
        encode_files
        find_empty_folders
    fi

    # If the argument is --decode, decode the files and folders
    if [ $1 = "--decode" ]; then
        decode_files
        decode_folders
        find_empty_folders
    fi
}

# Call main function
main $1

# Required packages, other than bash : sed, find, readlink, mkdir, mv, rmdir and everything else is bash builtins
