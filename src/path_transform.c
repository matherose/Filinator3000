/**
 * @file path_transform.c
 * @brief Path transformation implementation
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "file_ops.h"
#include "path_transform.h"

int path_transform(const char *in, char *out, size_t output_size, int encode, int is_directory) {
  int i = 0, j = 0;
  char current, transformed;
  
  assert(in != NULL);
  assert(out != NULL);
  assert(output_size > 0);
  
  /* Skip leading "./" or ".\" when decoding files */
  if (!is_directory && !encode && 
      (strncmp(in, "./", 2) == 0 || strncmp(in, ".\\", 2) == 0)) {
    i = 2;
  }

  /* Transform character by character */
  for (; in[i] != '\0' && j < (int)(output_size - 1); i++) {
    current = in[i];
    
    /* Handle different transformation based on type and mode */
    if (is_directory) {
      /* Directory transformation */
      transformed = encode ? (current == ' ' ? kSectionChar : current)
                          : (current == kSectionChar ? ' ' : current);
    } else if (encode) {
      /* File encoding */
      switch (current) {
        case '/':
        case '\\':
          transformed = kPathEncode;
          break;
        case ' ':
          transformed = kSpaceEncode;
          break;
        case kSectionChar:
          transformed = ' ';
          break;
        default:
          transformed = current;
          break;
      }
    } else {
      /* File decoding */
      switch (current) {
        case kPathEncode:
          transformed = PATH_SEP;
          break;
        case kSpaceEncode:
        case kSectionChar:
          transformed = ' ';
          break;
        default:
          transformed = current;
          break;
      }
    }
    
    out[j++] = transformed;
  }
  
  /* Ensure null termination */
  out[j] = '\0';

  /* Buffer overflow check */
  if (in[i] != '\0') {
    return -1; /* Buffer too small */
  }

  /* Post-processing */
  
  /* Remove leading path separator in decoded file paths */
  if (!is_directory && !encode && (out[0] == '/' || out[0] == '\\')) {
    memmove(out, out + 1, strlen(out));
  }
  
  /* Normalize path separators to platform format */
  if (!encode) {
    file_normalize_path(out);
  }
  
  return 0;
}

int path_process_file(const char *path, int encode, const char *output_dir) {
  char old_path[PATH_MAX], new_path[PATH_MAX], abs_path[PATH_MAX];
  char *slash;
  int len;

  assert(path != NULL);
  
  /* Prepare source path */
  strncpy(old_path, path, sizeof(old_path) - 1);
  old_path[sizeof(old_path) - 1] = '\0';

  /* Generate new path name based on encoding mode */
  if (encode) {
    /* For encoding, use absolute path */
    if (!file_get_absolute_path(old_path, abs_path)) {
      perror(old_path);
      return -1;
    }
    
    /* Transform the path */
    if (path_transform(abs_path, new_path, sizeof(new_path), 1, 0) != 0) {
      fprintf(stderr, "Path transformation failed: path too long\n");
      return -1;
    }
  } else {
    /* For decoding, use direct transformation */
    if (path_transform(old_path, new_path, sizeof(new_path), 0, 0) != 0) {
      fprintf(stderr, "Path transformation failed: path too long\n");
      return -1;
    }
    
    /* Create parent directories when decoding */
    slash = strrchr(new_path, PATH_SEP);
    if (slash) {
      len = (int)(slash - new_path);
      
      /* Create parent directory if path is valid */
      if (len > 0 && len < PATH_MAX) {
        char dir[PATH_MAX];
        strncpy(dir, new_path, len);
        dir[len] = '\0';
        if (file_mkpath(dir, 0755) != 0) {
          fprintf(stderr, "Failed to create directory: %s\n", dir);
          return -1;
        }
      }
    }
  }

  /* Skip if no change needed */
  if (strcmp(old_path, new_path) == 0) {
    return 0;
  }

  /* If output directory is specified, copy instead of rename */
  if (output_dir) {
    return path_process_file_output(path, output_dir);
  }
  
  /* Handle rename operation */
  if (platform_rename(old_path, new_path) != 0) {
    perror("rename (file)");
    return -1;
  }
  
  printf("Renamed: %s -> %s\n", old_path, new_path);
  return 0;
}

int path_process_file_output(const char *path, const char *output_dir) {
  char abs_path[PATH_MAX], enc_path[PATH_MAX], dest_path[PATH_MAX];
  char *slash;

  assert(path != NULL);
  assert(output_dir != NULL);
  
  /* Get absolute path */
  if (!file_get_absolute_path(path, abs_path)) {
    perror(path);
    return -1;
  }

  /* Encode the path */
  if (path_transform(abs_path, enc_path, sizeof(enc_path), 1, 0) != 0) {
    fprintf(stderr, "Path transformation failed: path too long\n");
    return -1;
  }
  
  /* Create destination path */
  if (platform_path_join(output_dir, enc_path, dest_path, sizeof(dest_path)) != 0) {
    fprintf(stderr, "Path join failed: path too long\n");
    return -1;
  }
  
  /* Create parent directories */
  slash = strrchr(dest_path, PATH_SEP);
  if (slash) {
    *slash = '\0';
    if (file_mkpath(dest_path, 0755) != 0) {
      fprintf(stderr, "Failed to create directory: %s\n", dest_path);
      return -1;
    }
    *slash = PATH_SEP;
  }

  /* Copy the file */
  if (file_copy(path, dest_path) == 0) {
    printf("Copied: %s -> %s\n", path, dest_path);
    return 0;
  } else {
    fprintf(stderr, "Failed to copy: %s -> %s\n", path, dest_path);
    return -1;
  }
}
