/**
 * @file path_transform.h
 * @brief Path transformation functions
 * 
 * Functions to encode or decode file and directory paths according to
 * Filinator's transformation rules.
 */

#ifndef PATH_TRANSFORM_H
#define PATH_TRANSFORM_H

/**
 * @brief Special characters used for encoding
 */
#define kSectionChar    '\xA7'  /**< Section character (§) used for encoding spaces in directory names */
#define kPathEncode     '@'     /**< Character used to encode path separators */
#define kSpaceEncode    '_'     /**< Character used to encode spaces in filenames */

/**
 * @brief Transform a path according to encoding rules
 *
 * @param in Input path
 * @param out Output buffer
 * @param output_size Size of output buffer
 * @param encode 1 for encoding, 0 for decoding
 * @param is_directory 1 for directory, 0 for file
 * @return 0 on success, -1 on failure
 */
int path_transform(const char *in, char *out, size_t output_size, int encode, int is_directory);

/**
 * @brief Process a single file
 *
 * @param path File path
 * @param encode Flag for encoding/decoding
 * @param output_dir Optional output directory (can be NULL)
 * @return 0 on success, -1 on failure
 */
int path_process_file(const char *path, int encode, const char *output_dir);

/**
 * @brief Process a file in output mode
 * 
 * @param path Source file path
 * @param output_dir Output directory
 * @return 0 on success, -1 on failure
 */
int path_process_file_output(const char *path, const char *output_dir);

#endif /* PATH_TRANSFORM_H */
