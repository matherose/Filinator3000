/**
 * @file platform.h
 * @brief Platform-specific abstractions and compatibility layer
 * 
 * This file provides cross-platform compatibility for directory and file operations
 * between Windows and UNIX-like systems.
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <sys/stat.h>
#include <limits.h>

/* Define PATH_MAX if not defined */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* Platform detection */
#ifdef _WIN32
  #include <windows.h>
  #include <direct.h>
  #define PLATFORM_WIN32 1
#else
  #include <dirent.h>
  #include <unistd.h>
  #define PLATFORM_UNIX 1
#endif

/* Platform-specific path separator */
#ifdef PLATFORM_WIN32
  #define PATH_SEP '\\'
  #define PATH_SEP_STR "\\"
#else
  #define PATH_SEP '/'
  #define PATH_SEP_STR "/"
#endif

/* Platform-specific file type checks */
#ifdef PLATFORM_WIN32
  #define IS_DIR(mode) ((mode) & _S_IFDIR)
  #define IS_REG_FILE(mode) ((mode) & _S_IFREG)
#else
  #define IS_DIR(mode) S_ISDIR(mode)
  #define IS_REG_FILE(mode) S_ISREG(mode)
#endif

/**
 * @brief Directory entry structure
 */
typedef struct {
  char d_name[PATH_MAX];  /**< Name of directory entry */
} platform_dirent_t;

/**
 * @brief Directory handle
 */
typedef struct platform_dir_t platform_dir_t;

/**
 * @brief Open a directory stream
 * 
 * @param path Directory path
 * @return Directory handle or NULL on failure
 */
platform_dir_t* platform_opendir(const char *path);

/**
 * @brief Read a directory entry
 * 
 * @param dir Directory stream
 * @return Directory entry or NULL when no more entries
 */
platform_dirent_t* platform_readdir(platform_dir_t *dir);

/**
 * @brief Close a directory stream
 * 
 * @param dir Directory stream to close
 * @return 0 on success, -1 on error
 */
int platform_closedir(platform_dir_t *dir);

/**
 * @brief Create a directory
 *
 * @param path Directory path
 * @param mode Permission mode (ignored on Windows)
 * @return 0 on success, -1 on error
 */
int platform_mkdir(const char *path, int mode);

/**
 * @brief Get absolute path from relative path
 *
 * @param path Input path
 * @param resolved_path Output buffer for absolute path
 * @return Pointer to resolved_path on success, NULL on failure
 */
char* platform_realpath(const char *path, char *resolved_path);

/**
 * @brief Rename a file or directory
 *
 * @param oldpath Old path
 * @param newpath New path
 * @return 0 on success, -1 on error
 */
int platform_rename(const char *oldpath, const char *newpath);

/**
 * @brief Build a full path by joining directory and filename
 *
 * @param dir Directory path
 * @param file Filename
 * @param result Buffer to store result
 * @param size Size of result buffer
 * @return 0 on success, -1 on error
 */
int platform_path_join(const char *dir, const char *file, char *result, size_t size);

#endif /* PLATFORM_H */
