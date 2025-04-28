/**
 * @file file_ops.h
 * @brief File operations for the Filinator tool
 */

#ifndef FILE_OPS_H
#define FILE_OPS_H

/**
 * @brief Copy a file with binary safety
 * 
 * @param src Source path
 * @param dst Destination path
 * @return 0 on success, -1 on failure
 */
int file_copy(const char *src, const char *dst);

/**
 * @brief Recursively create directories
 * 
 * @param path Directory path to create
 * @param mode Permission mode (ignored on Windows)
 * @return 0 on success, -1 on failure
 */
int file_mkpath(const char *path, int mode);

/**
 * @brief Get absolute path in a cross-platform manner
 * 
 * @param path Input path
 * @param absp Output buffer for absolute path
 * @return 1 on success, 0 on failure
 */
int file_get_absolute_path(const char *path, char *absp);

/**
 * @brief Normalize path separators to platform-specific ones
 * 
 * @param path Path to normalize (modified in-place)
 */
void file_normalize_path(char *path);

#endif /* FILE_OPS_H */
