/*
 * filinator.c - Optimized GNU99 version using recursive programming
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* Our encoded section character ('§') */
#define SEC_CHAR '\xA7'

/* Global variable: if not NULL, encode files by copying into this directory */
char *g_output_dir = NULL;

/* Recursively create directories (mkpath) */
int mkpath(const char *path, mode_t mode) {
    char tmp[PATH_MAX];
    char *p;
    size_t len;

    strncpy(tmp, path, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';

    len = strlen(tmp);
    if (len == 0) return -1;

    tmp[len-1] = (tmp[len-1] == '/') ? '\0' : tmp[len-1];

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

/* Transform a directory name or file path */
void transform_path(const char *in, char *out, int encode, int is_directory) {
    int i = 0, j = 0;

    // For file decoding, skip leading "./"
    if (!is_directory && !encode && strncmp(in, "./", 2) == 0)
        i = 2;

    for (; in[i] != '\0' && j < PATH_MAX - 1; i++) {
        if (is_directory) {
            out[j++] = encode ?
                      (in[i] == ' ' ? SEC_CHAR : in[i]) :
                      (in[i] == SEC_CHAR ? ' ' : in[i]);
        } else if (encode) {
            switch (in[i]) {
                case '/':      out[j++] = '@'; break;
                case ' ':      out[j++] = '_'; break;
                case SEC_CHAR: out[j++] = ' '; break;
                default:       out[j++] = in[i];
            }
        } else { // decode file path
            switch (in[i]) {
                case '@':      out[j++] = '/'; break;
                case SEC_CHAR:
                case '_':      out[j++] = ' '; break;
                default:       out[j++] = in[i];
            }
        }
    }
    out[j] = '\0';

    // Remove leading '/' in decoded file paths
    if (!is_directory && !encode && out[0] == '/')
        memmove(out, out + 1, strlen(out) + 1);
}

/* Copy a file from src to dst */
int copy_file(const char *src, const char *dst) {
    FILE *fsrc = NULL, *fdst = NULL;
    char buf[4096];
    size_t n;
    int status = 0;

    if (!(fsrc = fopen(src, "rb")))
        return -1;

    if (!(fdst = fopen(dst, "wb"))) {
        fclose(fsrc);
        return -1;
    }

    while ((n = fread(buf, 1, sizeof(buf), fsrc)) > 0)
        if (fwrite(buf, 1, n, fdst) != n) {
            status = -1;
            break;
        }

    fclose(fsrc);
    fclose(fdst);
    return status;
}

/* Process a single file */
void process_file(const char *path, int encode) {
    char oldp[PATH_MAX], newp[PATH_MAX], absp[PATH_MAX];

    strncpy(oldp, path, sizeof(oldp) - 1);
    oldp[sizeof(oldp) - 1] = '\0';

    if (encode) {
        if (!realpath(oldp, absp)) {
            perror(oldp);
            return;
        }
        transform_path(absp, newp, 1, 0);
    } else {
        transform_path(oldp, newp, 0, 0);

        // Ensure target directory exists for decode operation
        char *slash = strrchr(newp, '/');
        if (slash) {
            int len = slash - newp;
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

/* Process a single file in output mode (encode only) */
void process_file_out(const char *path) {
    char absp[PATH_MAX], enc[PATH_MAX], dest[PATH_MAX];

    if (!realpath(path, absp)) {
        perror(path);
        return;
    }

    transform_path(absp, enc, 1, 0);
    snprintf(dest, sizeof(dest), "%s/%s", g_output_dir, enc);

    if (copy_file(path, dest) == 0)
        printf("Copied: %s -> %s\n", path, dest);
}

/* Recursive directory processing */
void process_dir(const char *dpath, int encode, int skip_rename) {
    DIR *dp;
    struct dirent *de;
    struct stat st;
    char full[PATH_MAX];

    if (!(dp = opendir(dpath))) {
        perror(dpath);
        return;
    }

    while ((de = readdir(dp)) != NULL) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

        snprintf(full, sizeof(full), "%s/%s", dpath, de->d_name);

        if (stat(full, &st) != 0) {
            perror(full);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            process_dir(full, encode, 0);

            if (!g_output_dir && !skip_rename) {
                char newdir[PATH_MAX];
                transform_path(full, newdir, encode, 1);
                if (strcmp(full, newdir) != 0 && rename(full, newdir) != 0)
                    perror("rename (dir)");
            }
        } else if (S_ISREG(st.st_mode)) {
            if (g_output_dir && encode)
                process_file_out(full);
            else
                process_file(full, encode);
        }
    }

    closedir(dp);
}

int main(int argc, char *argv[]) {
    int encode;

    if (argc < 3) {
        fprintf(stderr, "Usage:\n  %s -encode <dir> [-output <dir>]\n  %s -decode <dir>\n",
                argv[0], argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-encode") == 0)
        encode = 1;
    else if (strcmp(argv[1], "-decode") == 0)
        encode = 0;
    else {
        fprintf(stderr, "Invalid mode\n");
        return 1;
    }

    if (encode && argc == 5 && strcmp(argv[3], "-output") == 0) {
        g_output_dir = argv[4];
        struct stat st;

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
        // Utilisation d'un dossier "output" par défaut pour l'encodage
        g_output_dir = "output";
        struct stat st;

        if (stat(g_output_dir, &st) != 0) {
            if (mkdir(g_output_dir, 0755) != 0) {
                perror("mkdir output");
                return 1;
            }
            printf("Dossier de sortie par défaut 'output' créé\n");
        } else if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "Le dossier de sortie par défaut 'output' existe déjà mais n'est pas un répertoire\n");
            return 1;
        }
    } else if (argc != 3) {
        fprintf(stderr, "Usage error\n");
        return 1;
    }

    process_dir(argv[2], encode, 1);
    return 0;
}
