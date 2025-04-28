/**
 * @file filinator.c
 * @brief Main implementation file for Filinator
 * 
 * Contains the main functionality for processing directories and files
 * according to Filinator's encoding/decoding rules.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "platform.h"
#include "file_ops.h"
#include "path_transform.h"
#include "filinator.h"

/**
 * @brief Process a directory entry (file or subdirectory)
 *
 * @param dir_path Parent directory path
 * @param entry_name Entry name
 * @param encode Flag for encoding/decoding
 * @param skip_rename Flag to skip renaming
 * @param output_dir Optional output directory (can be NULL)
 * @return 0 on success, non-zero on failure
 */
static int process_entry(const char *dir_path, const char *entry_name, 
                        int encode, int skip_rename, const char *output_dir) {
  struct stat st;
  char full_path[PATH_MAX];
  char new_dir[PATH_MAX];
  int result = 0;

  assert(dir_path != NULL);
  assert(entry_name != NULL);
  
  /* Skip special directory entries */
  if (strcmp(entry_name, ".") == 0 || strcmp(entry_name, "..") == 0) {
    return 0;
  }


  /* Construct full path */
  if (platform_path_join(dir_path, entry_name, full_path, sizeof(full_path)) != 0) {
    fprintf(stderr, "Path join failed: path too long\n");
    return -1;
  }

  /* Get file/directory info */
  if (stat(full_path, &st) != 0) {
    perror(full_path);
    return -1;
  }

  /* Process based on entry type */
  if (IS_REG_FILE(st.st_mode)) {
    /* Process regular file */
    if (output_dir && encode) {
      result = path_process_file_output(full_path, output_dir);
    } else {
      result = path_process_file(full_path, encode, NULL);
    }
    return result;
  }
  
  if (!IS_DIR(st.st_mode)) {
    /* Skip special files (symlinks, devices, etc.) */
    return 0;
  }
  
  /* Process directory recursively */
  result = filinator_process_directory(full_path, encode, 0, output_dir);
  if (result != 0) {
    return result;
  }
  
  /* Rename directory if needed */
  if (output_dir || skip_rename) {
    /* Skip renaming if we're in output mode or explicitly skipping */
    return 0;
  }
  
  /* Transform directory name */
  if (path_transform(full_path, new_dir, sizeof(new_dir), encode, 1) != 0) {
    fprintf(stderr, "Path transformation failed: path too long\n");
    return -1;
  }
  
  if (strcmp(full_path, new_dir) == 0) {
    /* No name change needed */
    return 0;
  }
  
  /* Rename the directory */
  if (platform_rename(full_path, new_dir) != 0) {
    perror("rename (dir)");
    return -1;
  }
  
  printf("Renamed directory: %s -> %s\n", full_path, new_dir);
  return 0;
}

/**
 * @brief Process a directory recursively
 */
int filinator_process_directory(const char *dir_path, int encode, 
                               int skip_rename, const char *output_dir) {
  platform_dir_t *dir;
  platform_dirent_t *entry;
  int result = 0;
  
  assert(dir_path != NULL);
  
  dir = platform_opendir(dir_path);
  if (dir == NULL) {
    perror(dir_path);
    return -1;
  }

  while ((entry = platform_readdir(dir)) != NULL) {
    int entry_result = process_entry(dir_path, entry->d_name, 
                                    encode, skip_rename, output_dir);
    
    /* Remember first failure but continue processing */
    if (entry_result != 0 && result == 0) {
      result = entry_result;
    }
  }

  platform_closedir(dir);
  return result;
}

/**
 * @brief Parse command line arguments and initialize configuration
 *
 * @param argc Argument count
 * @param argv Argument values
 * @param config Configuration structure to fill
 * @return 0 on success, non-zero on failure
 */
static int parse_arguments(int argc, char *argv[], filinator_config_t *config) {
  assert(argc >= 1);
  assert(argv != NULL);
  assert(config != NULL);
  
  /* Initialize config with default values */
  memset(config, 0, sizeof(filinator_config_t));
  
  /* Basic argument validation */
  if (argc < 3) {
    fprintf(stderr, "Usage:\n  %s -encode <dir> [-output <dir>]\n  %s -decode <dir>\n",
            argv[0], argv[0]);
    return -1;
  }
  
  /* Determine operation mode */
  if (strcmp(argv[1], "-encode") == 0) {
    config->mode = kModeEncode;
  } else if (strcmp(argv[1], "-decode") == 0) {
    config->mode = kModeDecode;
  } else {
    fprintf(stderr, "Invalid mode: %s\n", argv[1]);
    fprintf(stderr, "Usage:\n  %s -encode <dir> [-output <dir>]\n  %s -decode <dir>\n",
            argv[0], argv[0]);
    return -1;
  }
  
  /* Process arguments based on mode */
  switch (config->mode) {
    case kModeEncode:
      /* Handle different argument patterns for encode mode */
      config->input_dir = argv[2];
      
      if (argc == 3) {
        /* Default output directory */
        config->output_dir = "output";
      } else if (argc == 5 && strcmp(argv[3], "-output") == 0) {
        /* Custom output directory */
        config->output_dir = argv[4];
      } else {
        fprintf(stderr, "Invalid arguments for encode mode\n");
        fprintf(stderr, "Usage: %s -encode <dir> [-output <dir>]\n", argv[0]);
        return -1;
      }
      break;
      
    case kModeDecode:
      if (argc != 3) {
        fprintf(stderr, "Invalid arguments for decode mode\n");
        fprintf(stderr, "Usage: %s -decode <dir>\n", argv[0]);
        return -1;
      }
      /* Set input directory and no output directory in decode mode */
      config->input_dir = argv[2];
      config->output_dir = NULL;
      break;
  }
  
  return 0;
}

/**
 * @brief Ensure output directory exists
 *
 * @param output_dir Output directory path
 * @param is_default 1 if this is the default output directory
 * @return 0 on success, non-zero on failure
 */
static int ensure_output_directory(const char *output_dir, int is_default) {
  struct stat st;
  
  assert(output_dir != NULL);
  
  /* Check if directory exists */
  if (stat(output_dir, &st) != 0) {
    /* Directory doesn't exist, create it */
    if (file_mkpath(output_dir, 0755) != 0) {
      perror("Error creating output directory");
      return -1;
    }
    
    /* Inform about default directory creation */
    if (is_default) {
      printf("Default output directory '%s' created\n", output_dir);
    }
  } else if (!IS_DIR(st.st_mode)) {
    /* Path exists but is not a directory */
    fprintf(stderr, "'%s' exists but is not a directory\n", output_dir);
    return -1;
  }
  
  return 0;
}

/**
 * @brief Main program entry point
 */
int filinator_main(int argc, char *argv[]) {
  filinator_config_t config;
  int result;
  int is_default_output = 0;
  
  /* Parse command-line arguments */
  if (parse_arguments(argc, argv, &config) != 0) {
    return 1;
  }
  
  /* For encode mode with output directory */
  if (config.mode == kModeEncode && config.output_dir != NULL) {
    /* Check if using default output directory */
    is_default_output = (argc == 3);
    
    /* Ensure output directory exists */
    if (ensure_output_directory(config.output_dir, is_default_output) != 0) {
      return 1;
    }
  }
  
  /* Process the source directory */
  result = filinator_process_directory(
    config.input_dir,
    (config.mode == kModeEncode) ? 1 : 0,
    1,  /* Skip renaming the top-level directory */
    config.output_dir
  );
  
  return (result == 0) ? 0 : 1;
}

/* Main function - program entry point */
int main(int argc, char *argv[]) {
  return filinator_main(argc, argv);
}
