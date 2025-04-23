/*
 * filinator.c - File and directory name transformer
 * 
 * A POSIX-compliant C89 utility for transforming file and directory names
 * to improve portability across different systems and sharing platforms.
 * 
 * This program helps you safely share files with special characters
 * by encoding/decoding spaces and path separators.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

/* 
 * Define PATH_MAX if it's not defined
 * According to POSIX.1-2001, this is required for portability
 * across different systems and ensures safe path handling
 */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* Section character (§) used for encoding spaces in directory names */
#define SEC_CHAR '\xA7'

/* Global output directory path - when not NULL, encoded files are copied here */
char *g_output_dir = NULL;

/* 
 * Recursively create directories with specified permissions
 * Similar to mkdir -p on Unix systems
 * 
 * path: The directory path to create
 * mode: Permission mode (e.g. 0755 for rwxr-xr-x)
 * 
 * Returns: 0 on success, -1 on failure
 */
int mkpath(const char *path, mode_t mode) {
    char tmp[PATH_MAX];
    char *p;
    size_t len;

    strncpy(tmp, path, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';

    len = strlen(tmp);
    if (len == 0) return -1;

    /* Handle trailing slash by removing it */
    if (tmp[len-1] == '/')
        tmp[len-1] = '\0';

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, mode) != 0 && errno != EEXIST)
                return -1;
            *p = '/';
        }
    }
    return (mkdir(tmp, mode) != 0 && errno != EEXIST) ? -1 : 0;
}

/* 
 * Transform a file or directory path according to encoding rules
 * 
 * in: Input path to transform
 * out: Output buffer to receive transformed path
 * encode: 1 for encoding, 0 for decoding
 * is_directory: 1 if path is a directory, 0 if it's a file
 * 
 * Encoding rules:
 * - For directories: spaces → § (section character)
 * - For files: / → @, spaces → _, § → spaces
 */
void transform_path(const char *in, char *out, int encode, int is_directory) {
    int i = 0, j = 0;

    /* For file decoding, skip leading "./" to avoid path issues */
    if (!is_directory && !encode && strncmp(in, "./", 2) == 0)
        i = 2;

    for (; in[i] != '\0' && j < PATH_MAX - 1; i++) {
        if (is_directory) {
            out[j++] = encode ?
                      (in[i] == ' ' ? SEC_CHAR : in[i]) :
                      (in[i] == SEC_CHAR ? ' ' : in[i]);
        } else if (encode) {
            if (in[i] == '/') {
                out[j++] = '@';
            } else if (in[i] == ' ') {
                out[j++] = '_';
            } else if (in[i] == SEC_CHAR) {
                out[j++] = ' ';
            } else {
                out[j++] = in[i];
            }
        } else { /* decode file path */
            if (in[i] == '@') {
                out[j++] = '/';
            } else if (in[i] == '_' || in[i] == SEC_CHAR) {
                out[j++] = ' ';
            } else {
                out[j++] = in[i];
            }
        }
    }
    out[j] = '\0';

    /* Remove leading '/' in decoded file paths to prevent root-relative paths */
    if (!is_directory && !encode && out[0] == '/') {
        char *src = out + 1;
        char *dst = out;
        while ((*dst++ = *src++) != '\0')
            ;
    }
}

/* 
 * Copy a file from source to destination with binary safety
 * Uses buffer-based copy for efficiency
 * 
 * src: Source file path
 * dst: Destination file path
 * 
 * Returns: 0 on success, -1 on failure
 */
int copy_file(const char *src, const char *dst) {
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

/* 
 * Process a single file (rename it according to encoding/decoding rules)
 * 
 * path: Path to the file
 * encode: 1 for encoding, 0 for decoding
 */
void process_file(const char *path, int encode) {
    char oldp[PATH_MAX], newp[PATH_MAX], absp[PATH_MAX];
    char *slash;
    int len;

    strncpy(oldp, path, sizeof(oldp) - 1);
    oldp[sizeof(oldp) - 1] = '\0';

    if (encode) {
        /* For encoding, get absolute path first to ensure proper transformation */
        if (!realpath(oldp, absp)) {
            perror(oldp);
            return;
        }
        transform_path(absp, newp, 1, 0);
    } else {
        transform_path(oldp, newp, 0, 0);

        /* When decoding, ensure target directory exists before renaming */
        slash = strrchr(newp, '/');
        if (slash) {
            len = (int)(slash - newp);
            if (len < PATH_MAX) {
                char dir[PATH_MAX];
                strncpy(dir, newp, len);
                dir[len] = '\0';
                mkpath(dir, 0755);
            }
        }
    }

    if (strcmp(oldp, newp) != 0 && rename(oldp, newp) != 0)
        perror("rename (file)");
}

/* 
 * Process a file in output mode - encode and copy to output directory
 * 
 * path: Path to the source file
 */
void process_file_out(const char *path) {
    char absp[PATH_MAX], enc[PATH_MAX], dest[PATH_MAX];

    if (!realpath(path, absp)) {
        perror(path);
        return;
    }

    transform_path(absp, enc, 1, 0);
    sprintf(dest, "%s/%s", g_output_dir, enc);

    if (copy_file(path, dest) == 0)
        printf("Copied: %s -> %s\n", path, dest);
}

/* Forward declaration for recursive function */
void process_dir(const char *dpath, int encode, int skip_rename);

/* 
 * Process a single directory entry (file or subdirectory)
 * 
 * dpath: Parent directory path
 * name: Name of the entry (file or directory)
 * encode: 1 for encoding, 0 for decoding
 * skip_rename: 1 to skip renaming this entry (used for root directory)
 */
void process_entry(const char *dpath, const char *name, int encode, int skip_rename) {
    struct stat st;
    char full[PATH_MAX];
    char newdir[PATH_MAX];

    /* Construct full path by joining directory and entry name */
    sprintf(full, "%s/%s", dpath, name);

    if (stat(full, &st) != 0) {
        perror(full);
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        /* Process directory contents before renaming the directory itself */
        process_dir(full, encode, 0);

        if (!g_output_dir && !skip_rename) {
            transform_path(full, newdir, encode, 1);
            if (strcmp(full, newdir) != 0 && rename(full, newdir) != 0)
                perror("rename (dir)");
        }
    } else if (S_ISREG(st.st_mode)) {
        /* Handle files based on mode (output or in-place) */
        if (g_output_dir && encode)
            process_file_out(full);
        else
            process_file(full, encode);
    }
}

/* 
 * Recursively process a directory and its contents
 * 
 * dpath: Path to the directory
 * encode: 1 for encoding, 0 for decoding
 * skip_rename: 1 to skip renaming this directory (used for root directory)
 */
void process_dir(const char *dpath, int encode, int skip_rename) {
    DIR *dp;
    struct dirent *de;

    dp = opendir(dpath);
    if (dp == NULL) {
        perror(dpath);
        return;
    }

    while ((de = readdir(dp)) != NULL) {
        /* Skip special directory entries to avoid recursion problems */
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

        process_entry(dpath, de->d_name, encode, skip_rename);
    }

    closedir(dp);
}

int main(int argc, char *argv[]) {
    int encode;
    struct stat st;

    /* Check command line arguments */
    if (argc < 3) {
        fprintf(stderr, "Usage:\n  %s -encode <dir> [-output <dir>]\n  %s -decode <dir>\n",
                argv[0], argv[0]);
        return 1;
    }

    /* Determine operation mode */
    if (strcmp(argv[1], "-encode") == 0)
        encode = 1;
    else if (strcmp(argv[1], "-decode") == 0)
        encode = 0;
    else {
        fprintf(stderr, "Invalid mode\n");
        return 1;
    }

    /* Handle output directory specification for encoding mode */
    if (encode && argc == 5 && strcmp(argv[3], "-output") == 0) {
        g_output_dir = argv[4];

        if (stat(g_output_dir, &st) != 0) {
            if (mkdir(g_output_dir, 0755) != 0) {
                perror("mkdir output");
                return 1;
            }
        } else if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "Output is not a directory\n");
            return 1;
        }
    } else if (encode && argc == 3) {
        /* Use default "output" directory for encoding if none specified */
        g_output_dir = "output";

        if (stat(g_output_dir, &st) != 0) {
            if (mkdir(g_output_dir, 0755) != 0) {
                perror("mkdir output");
                return 1;
            }
            printf("Default output directory 'output' created\n");
        } else if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "Default output directory 'output' exists but is not a directory\n");
            return 1;
        }
    } else if (argc != 3) {
        fprintf(stderr, "Usage error\n");
        return 1;
    }

    /* Start processing from the specified root directory */
    process_dir(argv[2], encode, 1);
    return 0;
}
