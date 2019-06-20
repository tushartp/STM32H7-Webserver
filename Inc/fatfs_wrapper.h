/*
 * fatfs_wrapper.h
 *
 *  Created on: Apr 10, 2019
 *      Author: tcpatel
 */

#ifndef FATFS_WRAPPER_H_
#define FATFS_WRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "ff.h"

#if _USE_LFN != 0
#define NAME_MAX _MAX_LFN ///< Maximum size of a name in a file path
#else
#define MAX_NAME_LEN 13
#endif

// Opaque pointer representing files and directories.
typedef void *fs_file_t;
typedef void *fs_dir_t;

/* The following are dirent.h definitions are declared here to guarantee
 * consistency where structure may be different with different toolchains
 */
struct dirent {
    char d_name[NAME_MAX + 1]; ///< Name of file
    uint8_t d_type;          ///< Type of file
};

typedef struct dirent dirent_t;

enum {
    DT_UNKNOWN, ///< The file type could not be determined.
    DT_FIFO,    ///< This is a named pipe (FIFO).
    DT_CHR,     ///< This is a character device.
    DT_DIR,     ///< This is a directory.
    DT_BLK,     ///< This is a block device.
    DT_REG,     ///< This is a regular file.
    DT_LNK,     ///< This is a symbolic link.
    DT_SOCK,    ///< This is a UNIX domain socket.
};


struct statvfs {
    unsigned long  f_bsize;    ///< Filesystem block size
    unsigned long  f_frsize;   ///< Fragment size (block size)

    fsblkcnt_t     f_blocks;   ///< Number of blocks
    fsblkcnt_t     f_bfree;    ///< Number of free blocks
    fsblkcnt_t     f_bavail;   ///< Number of free blocks for unprivileged users

    unsigned long  f_fsid;     ///< Filesystem ID

    unsigned long  f_namemax;  ///< Maximum filename length
};

int fat_open(const char *path, int oflag, ... );
ssize_t fat_write(int fildes, const void *buf, size_t nbyte);
ssize_t fat_read(int fildes, void *buf, size_t nbyte);
off_t lseek(int fildes, off_t position, int whence);
int ftruncate(int fildes, off_t length);
int isatty(int fildes);
int fsync(int fildes);
int fstat(int fildes, struct stat *st);
//int poll(struct pollfd fds[], nfds_t nfds, int timeout);
int fat_close(int fildes);
int stat(const char *path, struct stat *st);
int statvfs(const char *path, struct statvfs *buf);

int chdir ( const char *pathname );
int chmod ( const char *pathname , mode_t mode );
char *getcwd ( char *pathname , size_t len );
int mkdir ( const char *pathname , mode_t mode );
int rename ( const char *oldpath , const char *newpath );
int rmdir ( const char *pathname );
int unlink ( const char *pathname );

DIR *opendir(const char *);
struct dirent *readdir(DIR *);
int closedir(DIR *);
void rewinddir(DIR *);
long telldir(DIR *);
void seekdir(DIR *, long);


#ifdef __cplusplus
}
#endif
#endif /* FATFS_WRAPPER_H_ */
