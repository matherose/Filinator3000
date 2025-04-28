/**
 * @file platform_unix.c
 * @brief UNIX-specific implementations of platform abstraction layer
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"

#ifdef PLATFORM_UNIX

/* UNIX-specific directory structure */
struct platform_dir_t {
  DIR *dir_handle;
};

platform_dir_t* platform_opendir(const char *path) {
  platform_dir_t *dir;
  
  assert(path != NULL);
  
  dir = (platform_dir_t*)malloc(sizeof(platform_dir_t));
  if (!dir) {
    return NULL;
  }
  
  dir->dir_handle = opendir(path);
  if (!dir->dir_handle) {
    free(dir);
    return NULL;
  }
  
  return dir;
}

platform_dirent_t* platform_readdir(platform_dir_t *dir) {
  static platform_dirent_t entry;
  struct dirent *native_entry;
  
  assert(dir != NULL);
  
  native_entry = readdir(dir->dir_handle);
  if (!native_entry) {
    return NULL;
  }
  
  strncpy(entry.d_name, native_entry->d_name, PATH_MAX - 1);
  entry.d_name[PATH_MAX - 1] = '\0';
  
  return &entry;
}

int platform_closedir(platform_dir_t *dir) {
  int result;
  
  assert(dir != NULL);
  
  result = closedir(dir->dir_handle);
  free(dir);
  
  return result;
}

int platform_mkdir(const char *path, int mode) {
  assert(path != NULL);
  
  return mkdir(path, (mode_t)mode);
}

char* platform_realpath(const char *path, char *resolved_path) {
  assert(path != NULL);
  assert(resolved_path != NULL);
  
  return realpath(path, resolved_path);
}

int platform_rename(const char *oldpath, const char *newpath) {
  assert(oldpath != NULL);
  assert(newpath != NULL);
  
  return rename(oldpath, newpath);
}

int platform_path_join(const char *dir, const char *file, char *result, size_t size) {
  assert(dir != NULL);
  assert(file != NULL);
  assert(result != NULL);
  
  if (snprintf(result, size, "%s/%s", dir, file) >= (int)size) {
    return -1;
  }
  
  return 0;
}

#endif /* PLATFORM_UNIX */
