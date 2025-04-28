/**
 * @file filinator.h
 * @brief Main header file for Filinator
 * 
 * A cross-platform C89 utility for transforming file and directory names
 * to improve portability across different systems and sharing platforms.
 * 
 * This program helps you safely share files with special characters
 * by encoding/decoding spaces and path separators.
 */

#ifndef FILINATOR_H
#define FILINATOR_H

/**
 * @brief Operation modes for the Filinator tool
 */
typedef enum {
  kModeEncode = 1,  /**< Encode mode */
  kModeDecode = 2   /**< Decode mode */
} filinator_mode_t;

/**
 * @brief Configuration options for the Filinator tool
 */
typedef struct {
  filinator_mode_t mode;    /**< Operation mode (encode or decode) */
  const char *input_dir;    /**< Input directory to process */
  const char *output_dir;   /**< Output directory (NULL for in-place) */
} filinator_config_t;

/**
 * @brief Process a directory recursively
 * 
 * @param dir_path Directory path
 * @param encode Flag for encoding/decoding
 * @param skip_rename Flag to skip renaming the directory itself
 * @param output_dir Optional output directory (can be NULL)
 * @return 0 on success, non-zero on failure
 */
int filinator_process_directory(const char *dir_path, int encode, 
                               int skip_rename, const char *output_dir);

/**
 * @brief Parse command line arguments and run Filinator
 * 
 * @param argc Argument count
 * @param argv Argument values
 * @return 0 on success, non-zero on failure
 */
int filinator_main(int argc, char *argv[]);

#endif /* FILINATOR_H */
