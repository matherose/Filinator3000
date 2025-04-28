/**
 * @file platform_win.c
 * @brief Windows-specific implementations of platform abstraction layer
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"

#ifdef PLATFORM_WIN32

/* Windows-specific directory structure */
struct platform_dir_t {
  HANDLE handle;
  WIN32_FIND_DATA find_data;
  int first_read;
};

platform_dir_t* platform_opendir(const char *path) {
  char search_path[PATH_MAX];
  platform_dir_t *dir;
  
  assert(path != NULL);
  
  dir = (platform_dir_t*)malloc(sizeof(platform_dir_t));
  if (!dir) {
    return NULL;
  }
  
  if (snprintf(search_path, PATH_MAX, "%s\\*", path) >= PATH_MAX) {
    free(dir);
    return NULL;
  }
  
  dir->handle = FindFirstFile(search_path, &dir->find_data);
  if (dir->handle == INVALID_HANDLE_VALUE) {
    free(dir);
    return NULL;
  }
  
  dir->first_read = 1;
  
  return dir;
}

platform_dirent_t* platform_readdir(platform_dir_t *dir) {
  static platform_dirent_t entry;
  
  assert(dir != NULL);
  
  if (dir->first_read) {
    dir->first_read = 0;
  } else if (!FindNextFile(dir->handle, &dir->find_data)) {
    return NULL;
  }
  
  strncpy(entry.d_name, dir->find_data.cFileName, PATH_MAX - 1);
  entry.d_name[PATH_MAX - 1] = '\0';
  
  return &entry;
}

int platform_closedir(platform_dir_t *dir) {
  assert(dir != NULL);
  
  FindClose(dir->handle);
  free(dir);
  
  return 0;
}

int platform_mkdir(const char *path, int mode) {
  assert(path != NULL);
  
  /* Mode is ignored in Windows */
  (void)mode;
  return _mkdir(path) ? -1 : 0;
}

char* platform_realpath(const char *path, char *resolved_path) {
  assert(path != NULL);
  assert(resolved_path != NULL);
  
  return _fullpath(resolved_path, path, PATH_MAX) ? resolved_path : NULL;
}

int platform_rename(const char *oldpath, const char *newpath) {
  assert(oldpath != NULL);
  assert(newpath != NULL);
  
  /* Windows often fails if target exists, so try to remove first */
  DeleteFile(newpath);
  
  return rename(oldpath, newpath) ? -1 : 0;
}

int platform_path_join(const char *dir, const char *file, char *result, size_t size) {
  assert(dir != NULL);
  assert(file != NULL);
  assert(result != NULL);
  
  if (snprintf(result, size, "%s\\%s", dir, file) >= (int)size) {
    return -1;
  }
  
  return 0;
}

#endif /* PLATFORM_WIN32 */
