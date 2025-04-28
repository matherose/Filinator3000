/**
 * @file file_ops.c
 * @brief File operations implementation for the Filinator tool
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "file_ops.h"

/**
 * @brief Copy a file with binary safety
 *
 * Implements a buffer-based copy that works for both text and binary files
 */
int file_copy(const char *src, const char *dst) {
  FILE *fsrc = NULL, *fdst = NULL;
  char buffer[4096];
  size_t bytes_read;
  int status = 0;

  assert(src != NULL);
  assert(dst != NULL);
  
  fsrc = fopen(src, "rb");
  if (fsrc == NULL) {
    return -1;
  }


  fdst = fopen(dst, "wb");
  if (fdst == NULL) {
    fclose(fsrc);
    return -1;
  }

  while ((bytes_read = fread(buffer, 1, sizeof(buffer), fsrc)) > 0) {
    if (fwrite(buffer, 1, bytes_read, fdst) != bytes_read) {
      status = -1;
      break;
    }
  }

  fclose(fsrc);
  fclose(fdst);
  return status;
}

/**
 * @brief Recursively create directories
 */
int file_mkpath(const char *path, int mode) {
  char temp_path[PATH_MAX];
  char *p;
  size_t len;

  assert(path != NULL);
  
  strncpy(temp_path, path, sizeof(temp_path) - 1);
  temp_path[sizeof(temp_path) - 1] = '\0';

  /* Remove trailing slash if present */
  len = strlen(temp_path);
  if (len == 0) {
    return -1;
  }
  
  if (temp_path[len-1] == PATH_SEP) {
    temp_path[len-1] = '\0';
  }

  /* Create each directory in the path */
  for (p = temp_path + 1; *p; p++) {
    if (*p == PATH_SEP) {
      *p = '\0';
      if (platform_mkdir(temp_path, mode) != 0 && errno != EEXIST) {
        return -1;
      }
      *p = PATH_SEP;
    }
  }

  /* Create the final directory */
  return (platform_mkdir(temp_path, mode) != 0 && errno != EEXIST) ? -1 : 0;
}

/**
 * @brief Get absolute path
 */
int file_get_absolute_path(const char *path, char *absp) {
  assert(path != NULL);
  assert(absp != NULL);
  
  return platform_realpath(path, absp) != NULL;
}

/**
 * @brief Normalize path separators
 */
void file_normalize_path(char *path) {
  char *p;
  
  assert(path != NULL);
  
  for (p = path; *p; p++) {
    if (*p == '/' || *p == '\\') {
      *p = PATH_SEP;
    }
  }
}
