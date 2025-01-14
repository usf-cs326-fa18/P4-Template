/**
 * netfs_client.h
 *
 * Implementation of the netfs client file system. Based on the fuse 'hello'
 * example here: https://github.com/libfuse/libfuse/blob/master/example/hello.c
 */

#define FUSE_USE_VERSION 31

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse3/fuse.h>
#include <netdb.h> 
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"
#include "logging.h"

#define TEST_DATA "hello world!\n"

/* Command line options */
static struct options {
    int show_help;
    int port;
} options;

#define OPTION(t, p) { t, offsetof(struct options, p), 1 }

/* Command line option specification. We can add more here. If we're interested
 * in a string, specify --opt=%s .*/
static const struct fuse_opt option_spec[] = {
    OPTION("-h", show_help),
    OPTION("--help", show_help),
    OPTION("--port=%d", port),
    FUSE_OPT_END
};

static int netfs_getattr(
        const char *path, struct stat *stbuf, struct fuse_file_info *fi) {

    LOG("getattr: %s\n", path);

    /* Clear the stat buffer */
    memset(stbuf, 0, sizeof(struct stat));

    /* By default, we will return 0 from this function (success) */
    int res = 0;

    if (strcmp(path, "/") == 0) {
        /* This is the root directory. We have hard-coded the permissions to 755
         * here, but you should apply the permissions from the remote directory
         * instead. The mode means:
         *   - S_IFDIR: this is a directory
         *   - 0755: user can read, write, execute. All others can read+execute.
         * The number of links refers to how many hard links point to the file.
         * If the link count reaches 0, the file is effectively deleted (this is
         * why deleting a file is actually 'unlinking' it).
         */
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else if (strcmp(path+1, "test_file") == 0) {
        /* Incrementing the path pointer by 1 will remove the '/' from the start
         * of the path. We're comparing it with a hard-coded file name.
         *   - S_IFREG: indicates a regular file
         * We also hard-code the size of this file based on its contents: 'hello
         * world!' */
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(TEST_DATA);
    } else {
        /* -ENOENT = 'no such file or directory'. In other words, this demo code
         * only supports the root directory and a single file named
         * "test_file" */
        res = -ENOENT;
    }

    return res;
}

static int netfs_readdir(
        const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
        struct fuse_file_info *fi, enum fuse_readdir_flags flags) {

    LOG("readdir: %s\n", path);

    /* By default, we will return 0 from this function (success) */
    int res = 0;

    /* We only support one directory: the root directory. */
    if (strcmp(path, "/") != 0) {
        return -ENOENT;
    } else {
        /* Create our . and .. directory links */
        filler(buf, ".", NULL, 0, 0);
        filler(buf, "..", NULL, 0, 0);

        /* Create our single test file */
        filler(buf, "test_file", NULL, 0, 0);
    }

    return res;
}

static int netfs_open(const char *path, struct fuse_file_info *fi) {

    LOG("open: %s\n", path);

    /* By default, we will return 0 from this function (success) */
    int res = 0;

    /* Once again, incrementing the path pointer by 1 will remove the '/' from
     * the start of the path. We compare it with our test file. */
    if (strcmp(path+1, "test_file") != 0) {
        return -ENOENT;
    }

    /* We only support opening the file in read-only mode */
    if ((fi->flags & O_ACCMODE) != O_RDONLY) {
        return -EACCES;
    }

    return res;
}

static int netfs_read(
        const char *path, char *buf, size_t size, off_t offset,
        struct fuse_file_info *fi) {

    LOG("read: %s\n", path);

    if(strcmp(path+1, "test_file") != 0) {
        /* We only support one file (test_file)...*/
        return -ENOENT;
    }

    size_t len;
    len = strlen(TEST_DATA);
    if (offset < len) {
        if (offset + size > len) {
            size = len - offset;
        }
        /* Note how the read request may not start at the beginning of the file.
         * We take the offset into account here: */
        memcpy(buf, TEST_DATA + offset, size);
    } else {
        size = 0;
    }

    return size;
}

/* This struct maps file system operations to our custom functions defined
 * above. */
static struct fuse_operations netfs_client_ops = {
    .getattr = netfs_getattr,
    .readdir = netfs_readdir,
    .open = netfs_open,
    .read = netfs_read,
};

static void show_help(char *argv[]) {
    printf("usage: %s [options] <mountpoint>\n\n", argv[0]);
    printf("File-system specific options:\n"
            "    --port=<n>          Port number to connect to\n"
            "                        (default: %d)"
            "\n", DEFAULT_PORT);
}

int main(int argc, char *argv[]) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    /* Set up default options: */
    options.port = DEFAULT_PORT;

    /* Parse options */
    if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1) {
        return 1;
    }

    if (options.show_help) {
        show_help(argv);
        assert(fuse_opt_add_arg(&args, "--help") == 0);
        args.argv[0] = (char*) "";
    }

    return fuse_main(args.argc, args.argv, &netfs_client_ops, NULL);
}
