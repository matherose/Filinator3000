/**
 * @file filinator.c
 * @brief File and directory name transformer
 * 
 * A cross-platform C89 utility for transforming file and directory names
 * to improve portability across different systems and sharing platforms.
 * 
 * This program helps you safely share files with special characters
 * by encoding/decoding spaces and path separators.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <limits.h>

/* Platform-specific includes and defines */
#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define mkdir(path, mode)      _mkdir(path)
    #define PATH_SEP               '\\'
    #define PATH_SEP_STR           "\\"
    #define realpath(N,R)          _fullpath((R),(N),PATH_MAX)
    #define IS_DIR(mode)           ((mode) & _S_IFDIR)
    #define IS_REG_FILE(mode)      ((mode) & _S_IFREG)
#else
    #include <dirent.h>
    #include <unistd.h>
    #define PATH_SEP               '/'
    #define PATH_SEP_STR           "/"
    #define IS_DIR(mode)           S_ISDIR(mode)
    #define IS_REG_FILE(mode)      S_ISREG(mode)
#endif

/* Define PATH_MAX if not defined */
#ifndef PATH_MAX
    #define PATH_MAX 4096
#endif

/* Special characters used for encoding */
#define SEC_CHAR    '\xA7'  /* Section character (§) used for encoding spaces in directory names */
#define PATH_ENCODE '@'     /* Character used to encode path separators */
#define SPACE_ENCODE '_'    /* Character used to encode spaces in filenames */

/* Global output directory path */
static char *g_output_dir = NULL;

/* Forward declarations */
static void process_dir(const char *dpath, int encode, int skip_rename);

/**
 * @name Windows Directory API Compatibility Layer
 * @{
 */
#ifdef _WIN32
typedef struct {
    HANDLE handle;          /**< Windows directory handle */
    WIN32_FIND_DATA data;   /**< Windows file information structure */
    int first;              /**< Flag indicating first read operation */
} DIR;

struct dirent {
    char d_name[MAX_PATH];  /**< Name of directory entry */
};

/**
 * @brief Open a directory stream
 * 
 * @param path Directory path
 * @return DIR pointer or NULL on failure
 */
static DIR *opendir(const char *path) 
{
    DIR *dir = malloc(sizeof(DIR));
    char search_path[PATH_MAX];
    
    if (!dir) 
        return NULL;
    
    snprintf(search_path, PATH_MAX, "%s\\*", path);
    dir->handle = FindFirstFile(search_path, &dir->data);
    
    if (dir->handle == INVALID_HANDLE_VALUE) {
        free(dir);
        return NULL;
    }
    
    dir->first = 1;
    return dir;
}

/**
 * @brief Read a directory entry
 * 
 * @param dir Directory stream
 * @return Directory entry or NULL when no more entries
 */
static struct dirent *readdir(DIR *dir) 
{
    static struct dirent entry;
    
    if (dir->first)
        dir->first = 0;
    else if (!FindNextFile(dir->handle, &dir->data))
        return NULL;
    
    strcpy(entry.d_name, dir->data.cFileName);
    return &entry;
}

/**
 * @brief Close a directory stream
 * 
 * @param dir Directory stream to close
 * @return 0 on success
 */
static int closedir(DIR *dir) 
{
    if (dir) {
        FindClose(dir->handle);
        free(dir);
    }
    return 0;
}
#endif /* _WIN32 */
/** @} */

/**
 * @brief Recursively create directories
 * 
 * @param path Directory path to create
 * @param mode Permission mode (ignored on Windows)
 * @return 0 on success, -1 on failure
 */
static int mkpath(const char *path, mode_t mode) 
{
    char tmp[PATH_MAX];
    char *p;
    size_t len;

    strncpy(tmp, path, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';

    /* Remove trailing slash if present */
    len = strlen(tmp);
    if (len == 0) 
        return -1;
    if (tmp[len-1] == PATH_SEP)
        tmp[len-1] = '\0';

    /* Create each directory in the path */
    for (p = tmp + 1; *p; p++) {
        if (*p == PATH_SEP) {
            *p = '\0';
            #ifdef _WIN32
            if (_mkdir(tmp) != 0 && errno != EEXIST)
            #else
            if (mkdir(tmp, mode) != 0 && errno != EEXIST)
            #endif
                return -1;
            *p = PATH_SEP;
        }
    }

    /* Create the final directory */
    #ifdef _WIN32
    return (_mkdir(tmp) != 0 && errno != EEXIST) ? -1 : 0;
    #else
    return (mkdir(tmp, mode) != 0 && errno != EEXIST) ? -1 : 0;
    #endif
}

/**
 * @brief Normalize path separators to platform-specific ones
 * 
 * @param path Path to normalize (modified in-place)
 */
static void normalize_path(char *path) 
{
    char *p;
    for (p = path; *p; p++) {
        if (*p == '/' || *p == '\\')
            *p = PATH_SEP;
    }
}

/**
 * @brief Transform a path according to encoding rules
 * 
 * @param in Input path
 * @param out Output buffer 
 * @param encode 1 for encoding, 0 for decoding
 * @param is_directory 1 for directory, 0 for file
 */
static void transform_path(const char *in, char *out, int encode, int is_directory) 
{
    int i = 0, j = 0;
    char current, transformed;
    
    /* Skip leading "./" or ".\" when decoding files */
    if (!is_directory && !encode && 
        (strncmp(in, "./", 2) == 0 || strncmp(in, ".\\", 2) == 0))
        i = 2;

    /* Transform character by character */
    for (; in[i] != '\0' && j < PATH_MAX - 1; i++) {
        current = in[i];
        
        /* Handle different transformation based on type and mode */
        if (is_directory) {
            /* Directory transformation */
            transformed = encode ? (current == ' ' ? SEC_CHAR : current)
                                : (current == SEC_CHAR ? ' ' : current);
        } else if (encode) {
            /* File encoding */
            switch (current) {
                case '/':
                case '\\':
                    transformed = PATH_ENCODE;
                    break;
                case ' ':
                    transformed = SPACE_ENCODE;
                    break;
                case SEC_CHAR:
                    transformed = ' ';
                    break;
                default:
                    transformed = current;
                    break;
            }
        } else {
            /* File decoding */
            switch (current) {
                case PATH_ENCODE:
                    transformed = PATH_SEP;
                    break;
                case SPACE_ENCODE:
                case SEC_CHAR:
                    transformed = ' ';
                    break;
                default:
                    transformed = current;
                    break;
            }
        }
        
        out[j++] = transformed;
    }
    out[j] = '\0';

    /* Post-processing */
    
    /* Remove leading path separator in decoded file paths */
    if (!is_directory && !encode && (out[0] == '/' || out[0] == '\\'))
        memmove(out, out + 1, strlen(out));
    
    /* Normalize path separators to platform format */
    if (!encode)
        normalize_path(out);
}

/**
 * @brief Copy a file with binary safety
 * 
 * @param src Source path
 * @param dst Destination path
 * @return 0 on success, -1 on failure
 */
static int copy_file(const char *src, const char *dst) 
{
    FILE *fsrc = NULL, *fdst = NULL;
    char buf[4096];
    size_t n;
    int status = 0;

    fsrc = fopen(src, "rb");
    if (fsrc == NULL)
        return -1;

    fdst = fopen(dst, "wb");
    if (fdst == NULL) {
        fclose(fsrc);
        return -1;
    }

    while ((n = fread(buf, 1, sizeof(buf), fsrc)) > 0) {
        if (fwrite(buf, 1, n, fdst) != n) {
            status = -1;
            break;
        }
    }

    fclose(fsrc);
    fclose(fdst);
    return status;
}

/**
 * @brief Get absolute path in a cross-platform manner
 * 
 * @param path Input path
 * @param absp Output buffer for absolute path
 * @return 1 on success, 0 on failure
 */
static int get_absolute_path(const char *path, char *absp) 
{
    #ifdef _WIN32
    return _fullpath(absp, path, PATH_MAX) != NULL;
    #else
    return realpath(path, absp) != NULL;
    #endif
}

/**
 * @brief Process a single file
 * 
 * @param path File path
 * @param encode Flag for encoding/decoding
 */
static void process_file(const char *path, int encode) 
{
    char oldp[PATH_MAX], newp[PATH_MAX], absp[PATH_MAX];
    char *slash;
    int len;

    /* Prepare source path */
    strncpy(oldp, path, sizeof(oldp) - 1);
    oldp[sizeof(oldp) - 1] = '\0';

    /* Generate new path name based on encoding mode */
    if (encode) {
        /* For encoding, use absolute path */
        if (!get_absolute_path(oldp, absp)) {
            perror(oldp);
            return;
        }
        transform_path(absp, newp, 1, 0);
    } else {
        /* For decoding, use direct transformation */
        transform_path(oldp, newp, 0, 0);
        
        /* Create parent directories when decoding */
        slash = strrchr(newp, PATH_SEP);
        if (slash) {
            len = (int)(slash - newp);
            
            /* Create parent directory if path is valid */
            if (len < PATH_MAX) {
                char dir[PATH_MAX];
                strncpy(dir, newp, len);
                dir[len] = '\0';
                mkpath(dir, 0755);
            }
        }
    }

    /* Skip if no change needed */
    if (strcmp(oldp, newp) == 0)
        return;

    /* Handle rename operation */
    #ifdef _WIN32
    /* Handle Windows rename restrictions */
    remove(newp);
    #endif
    
    if (rename(oldp, newp) != 0)
        perror("rename (file)");
}

/**
 * @brief Process a file in output mode
 * 
 * @param path Source file path
 */
static void process_file_out(const char *path) 
{
    char absp[PATH_MAX], enc[PATH_MAX], dest[PATH_MAX];

    /* Get absolute path */
    if (!get_absolute_path(path, absp)) {
        perror(path);
        return;
    }

    /* Encode the path */
    transform_path(absp, enc, 1, 0);
    
    /* Create destination path */
    #ifdef _WIN32
    sprintf(dest, "%s\\%s", g_output_dir, enc);
    #else
    sprintf(dest, "%s/%s", g_output_dir, enc);
    #endif

    /* Copy the file */
    if (copy_file(path, dest) == 0)
        printf("Copied: %s -> %s\n", path, dest);
}

/**
 * @brief Process a directory entry (file or subdirectory)
 * 
 * @param dpath Parent directory path
 * @param name Entry name
 * @param encode Flag for encoding/decoding
 * @param skip_rename Flag to skip renaming
 */
static void process_entry(const char *dpath, const char *name, int encode, int skip_rename) 
{
    struct stat st;
    char full[PATH_MAX];
    char newdir[PATH_MAX];

    /* Skip special directory entries */
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
        return;

    /* Construct full path */
    #ifdef _WIN32
    sprintf(full, "%s\\%s", dpath, name);
    #else
    sprintf(full, "%s/%s", dpath, name);
    #endif

    /* Get file/directory info */
    if (stat(full, &st) != 0) {
        perror(full);
        return;
    }

    /* Process based on entry type */
    if (IS_REG_FILE(st.st_mode)) {
        /* Process regular file */
        if (g_output_dir && encode)
            process_file_out(full);
        else
            process_file(full, encode);
        return;
    }
    
    if (!IS_DIR(st.st_mode))
        /* Skip special files (symlinks, devices, etc.) */
        return;
    
    /* Process directory */
    process_dir(full, encode, 0);
        
    /* Rename directory if needed */
    if (g_output_dir || skip_rename)
        /* Skip renaming if we're in output mode or explicitly skipping */
        return;
        
    transform_path(full, newdir, encode, 1);
    if (strcmp(full, newdir) == 0)
        /* No name change needed */
        return;
        
    #ifdef _WIN32
    /* On Windows, rename fails if target exists */
    rmdir(newdir);
    #endif
        
    if (rename(full, newdir) != 0)
        perror("rename (dir)");
}

/**
 * @brief Process a directory recursively
 * 
 * @param dpath Directory path
 * @param encode Flag for encoding/decoding
 * @param skip_rename Flag to skip renaming the directory itself
 */
static void process_dir(const char *dpath, int encode, int skip_rename) 
{
    DIR *dp;
    struct dirent *de;

    dp = opendir(dpath);
    if (dp == NULL) {
        perror(dpath);
        return;
    }

    while ((de = readdir(dp)) != NULL)
        process_entry(dpath, de->d_name, encode, skip_rename);

    closedir(dp);
}

/**
 * @brief Main program entry point
 * 
 * @param argc Argument count
 * @param argv Argument values
 * @return Exit status
 */
int main(int argc, char *argv[]) 
{
    int encode;
    struct stat st;
    int mode;
    
    /* Basic argument validation */
    if (argc < 3) {
        fprintf(stderr, "Usage:\n  %s -encode <dir> [-output <dir>]\n  %s -decode <dir>\n",
                argv[0], argv[0]);
        return 1;
    }
    
    /* Determine operation mode using a mode identifier */
    if (strcmp(argv[1], "-encode") == 0) {
        mode = 1;  /* encode mode */
        encode = 1;
    } else if (strcmp(argv[1], "-decode") == 0) {
        mode = 2;  /* decode mode */
        encode = 0;
    } else {
        fprintf(stderr, "Invalid mode: %s\n", argv[1]);
        fprintf(stderr, "Usage:\n  %s -encode <dir> [-output <dir>]\n  %s -decode <dir>\n",
                argv[0], argv[0]);
        return 1;
    }
    
    /* Process arguments based on mode */
    switch (mode) {
        case 1:  /* encode mode */
            /* Handle different argument patterns for encode mode */
            if (argc == 3)
                /* Default output directory */
                g_output_dir = "output";
            else if (argc == 5 && strcmp(argv[3], "-output") == 0)
                /* Custom output directory */
                g_output_dir = argv[4];
            else {
                fprintf(stderr, "Invalid arguments for encode mode\n");
                fprintf(stderr, "Usage: %s -encode <dir> [-output <dir>]\n", argv[0]);
                return 1;
            }
            
            /* Ensure output directory exists */
            if (stat(g_output_dir, &st) != 0) {
                /* Directory doesn't exist, create it */
                if (mkdir(g_output_dir, 0755) != 0) {
                    perror("Error creating output directory");
                    return 1;
                }
                
                /* Inform about default directory creation */
                if (argc == 3)
                    printf("Default output directory '%s' created\n", g_output_dir);
            } else if (!IS_DIR(st.st_mode)) {
                /* Path exists but is not a directory */
                fprintf(stderr, "'%s' exists but is not a directory\n", g_output_dir);
                return 1;
            }
            break;
            
        case 2:  /* decode mode */
            if (argc != 3) {
                fprintf(stderr, "Invalid arguments for decode mode\n");
                fprintf(stderr, "Usage: %s -decode <dir>\n", argv[0]);
                return 1;
            }
            /* No output directory in decode mode */
            g_output_dir = NULL;
            break;
    }
    
    /* Process the source directory */
    process_dir(argv[2], encode, 1);
    return 0;
}
