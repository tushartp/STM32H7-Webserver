/*
 * fatfs_wrapper.c
 *
 *  Created on: Apr 10, 2019
 *      Author: tcpatel
 */


#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "cmsis_os.h"
#include "fatfs_wrapper.h"



static FIL *FileHandle[FOPEN_MAX];
static osMutexId filehandle_mutex = NULL;

/*
 * POSIX <--> FatFS bridge functions
 * 	-fatfs_file_handle
 * 	-fileno_to_fatfs
 * 	-fatfs_to_errno
 * 	-free_file_descriptor
 * 	-new_file_descriptor
 *
 * POSIX wrapper character I/O functions
 *	-isatty
 *
 * POSIX wrapper file positions functions
 * 	-lseek
 *
 *
 * POSIX wrapper file functions
 * 	-open
 * 	-write
 * 	-read
 * 	-fsync
 * 	-ftruncate
 * 	-truncate
 * 	-close
 *
 * POSIX wraper file information function
 * 	-fstat
 * 	-stat
 *
 * POSIX file and directory manipulation
 *	-chdir
 *	-chmod
 *	-getcwd
 *	-mkdir
 *	-rename
 *	-rmdir
 *	-unlink
 *
 *	POSIX - directory scanning functions
 *	-closedir
 *	-opendir
 *	-readdir
 *
 *
 */


FIL *fatfs_file_handle(int fd)
{
    if (fd >= FOPEN_MAX) {
        return NULL;
    }
    FIL *fh = FileHandle[fd];

    return fh;
}




/// @brief  Convert POSIX fileno to FatFS handle
/// NOT POSIX
///
/// - FatFS file handle is pointed to by the avr-libc stream->udata.
///
/// @param[in] fileno: fileno of file
///
/// @return FIL * FatFS file handle on success.
/// @return NULL if POSIX fileno is invalid NULL

FIL *fileno_to_fatfs(int fileno)
{
    FIL *fh;

    if(isatty( fileno ))
    {
        errno = EBADF;
        return(NULL);
    }

    fh = fatfs_file_handle(fileno);
    if(fh == NULL)
    {
        errno = EBADF;
        return(NULL);
    }
    return(fh);
}



/// @brief Convert FafFs error result to POSIX errno.
/// NOT POSIX
///
/// - man page errno (3).
///
/// @param[in] Result: FatFs Result code.
///
/// @return POSIX errno.
/// @return EBADMSG if no conversion possible.
int fatfs_to_errno( FRESULT Result )
{
    switch( Result )
    {
        case FR_OK:              /* FatFS (0) Succeeded */
            return (0);          /* POSIX OK */
        case FR_DISK_ERR:        /* FatFS (1) A hard error occurred in the low level disk I/O layer */
            return (EIO);        /* POSIX Input/output error (POSIX.1) */

        case FR_INT_ERR:         /* FatFS (2) Assertion failed */
            return (EPERM);      /* POSIX Operation not permitted (POSIX.1) */

        case FR_NOT_READY:       /* FatFS (3) The physical drive cannot work */
            return (EBUSY);      /* POSIX Device or resource busy (POSIX.1) */

        case FR_NO_FILE:         /* FatFS (4) Could not find the file */
            return (ENOENT);     /* POSIX No such file or directory (POSIX.1) */

        case FR_NO_PATH:         /* FatFS (5) Could not find the path */
            return (ENOENT);     /* POSIX No such file or directory (POSIX.1) */

        case FR_INVALID_NAME:    /* FatFS (6) The path name format is invalid */
            return (EINVAL);     /* POSIX Invalid argument (POSIX.1) */

        case FR_DENIED:          /* FatFS (7) Access denied due to prohibited access or directory full */
            return (EACCES);     /* POSIX Permission denied (POSIX.1) */
        case FR_EXIST:           /* FatFS (8) Access denied due to prohibited access */
            return (EACCES);     /* POSIX Permission denied (POSIX.1) */

        case FR_INVALID_OBJECT:  /* FatFS (9) The file/directory object is invalid */
            return (EINVAL);     /* POSIX Invalid argument (POSIX.1) */

        case FR_WRITE_PROTECTED: /* FatFS (10) The physical drive is write protected */
            return(EROFS);       /* POSIX Read-only filesystem (POSIX.1) */

        case FR_INVALID_DRIVE:   /* FatFS (11) The logical drive number is invalid */
            return(ENXIO);       /* POSIX No such device or address (POSIX.1) */

        case FR_NOT_ENABLED:     /* FatFS (12) The volume has no work area */
            return (ENOSPC);     /* POSIX No space left on device (POSIX.1) */

        case FR_NO_FILESYSTEM:   /* FatFS (13) There is no valid FAT volume */
            return(ENXIO);       /* POSIX No such device or address (POSIX.1) */

        case FR_MKFS_ABORTED:    /* FatFS (14) The f_mkfs() aborted due to any parameter error */
            return (EINVAL);     /* POSIX Invalid argument (POSIX.1) */

        case FR_TIMEOUT:         /* FatFS (15) Could not get a grant to access the volume within defined period */
            return (EBUSY);      /* POSIX Device or resource busy (POSIX.1) */

        case FR_LOCKED:          /* FatFS (16) The operation is rejected according to the file sharing policy */
            return (EBUSY);      /* POSIX Device or resource busy (POSIX.1) */


        case FR_NOT_ENOUGH_CORE: /* FatFS (17) LFN working buffer could not be allocated */
            return (ENOMEM);     /* POSIX Not enough space (POSIX.1) */

        case FR_TOO_MANY_OPEN_FILES:/* FatFS (18) Number of open files > _FS_SHARE */
            return (EMFILE);     /* POSIX Too many open files (POSIX.1) */

        case FR_INVALID_PARAMETER:/* FatFS (19) Given parameter is invalid */
            return (EINVAL);     /* POSIX Invalid argument (POSIX.1) */

    }
    return (EBADMSG);            /* POSIX Bad message (POSIX.1) */
}

int free_file_descriptor(int fileno)
{
    FIL *fh;

    if(isatty( fileno ))
    {
        errno = EBADF;
        return(-1);
    }
    osMutexWait(filehandle_mutex, osWaitForever);

    fh = FileHandle[fileno];
    ff_free(fh);
    FileHandle[fileno]  = NULL;

    osMutexRelease(filehandle_mutex);
    return(fileno);
}

// =============================================
/// @brief Allocate a POSIX FILE descriptor.
/// NOT POSIX
///
/// @return fileno on success.
/// @return -1 on failure with errno set.
int new_file_descriptor( void )
{
    int i;
    FIL *fh;

    if(filehandle_mutex == NULL)
    {
    	osMutexDef(file);
    	filehandle_mutex = osMutexCreate(osMutex(file));
    }


    osMutexWait(filehandle_mutex, osWaitForever);

		for(i=0;i<FOPEN_MAX;++i)
		{
			if(isatty(i))
				continue;
			if( FileHandle[i] == NULL)
			{

				fh = (FIL *) ff_malloc(sizeof(FIL));
				if(fh == NULL)
				{
					errno = ENOMEM;
					return(-1);
				}

				FileHandle[i]  = fh;
				osMutexRelease(filehandle_mutex);
				return(i);
			}
		}

    errno = ENFILE;
    osMutexRelease(filehandle_mutex);
    return(-1);
}

int isatty(int fileno)
{
///  Perhaps we should verify console functions have been added ?
    if(fileno >= 0 && fileno <= 2)
        return(1);
    return 0;
}


off_t lseek(int fildes, off_t position, int whence)
{
	FRESULT res;
	FIL *fh = fileno_to_fatfs(fildes);
	errno = 0;

	if(fh == NULL)
	{
		errno = EMFILE;
		return -1;
	}

	if(isatty(fildes))
	{
		return (-1);
	}



    if(whence == SEEK_END)
        position += f_size(fh);
    else if(whence==SEEK_CUR)
        position += fh->fptr;

    res = f_lseek(fh, position);
    if(res)
    {
        errno = fatfs_to_errno(res);
        return -1;
    }
    if(position != f_tell(fh) )
    {
        //("Seek failed");
        return(-1L);
    }
    return (fh->fptr);
}

int fat_open(const char *path, int oflag, ... )
{
    int fileno;
    FIL *fh;
    int res;

    errno = 0;
    /* POSIX flags -> FatFS open mode */
	int flags = oflag;
	flags &= ~O_BINARY;

    BYTE openmode;
    if (flags & O_RDWR) {
        openmode = FA_READ | FA_WRITE;
    } else if (flags & O_WRONLY) {
        openmode = FA_WRITE;
    } else {
        openmode = FA_READ;
    }

    if (flags & O_CREAT) {
        if (flags & O_TRUNC) {
            openmode |= FA_CREATE_ALWAYS;
        } else {
            openmode |= FA_OPEN_ALWAYS;
        }
    }

    if (flags & O_APPEND) {
        openmode |= FA_OPEN_APPEND;
    }

    fileno = new_file_descriptor();

    if(fileno == -1)
    {
    	return (-1);
    }

    fh = fileno_to_fatfs(fileno);

    if(fh == NULL)
    {
        free_file_descriptor(fileno);
        errno = EBADF;
        return(-1);
    }

    res = f_open(fh, path, openmode);
    if(res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        free_file_descriptor(fileno);
        return(-1);
    }

    if(flags & O_APPEND)
    {
///  Seek to end of the file
        res = f_lseek(fh, f_size(fh));
        if (res != FR_OK)
        {
            errno = fatfs_to_errno(res);
            f_close(fh);
            free_file_descriptor(fileno);
            return(-1);
        }
    }

    return(fileno);
}



ssize_t fat_write(int fildes, const void *buf, size_t nbyte)
{
	FRESULT res;
    UINT size;
    UINT bytes = nbyte;
	FIL *fh = fileno_to_fatfs(fildes);
	errno = 0;

	if(fh == NULL)
	{
		errno = EBADF;
		return -1;
	}
	res = f_write(fh, buf, bytes, &size);
    if(res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(-1);
    }
    return ((ssize_t) size);
}


ssize_t fat_read(int fildes, void *buf, size_t nbyte)
{
	FRESULT res;
    UINT size;
    UINT bytes = nbyte;
	FIL *fh = fileno_to_fatfs(fildes);
	errno = 0;

	if(fh == NULL)
	{
		errno = EBADF;
		return -1;
	}
	res = f_read(fh, buf, bytes, &size);
    if(res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(-1);
    }
    return ((ssize_t) size);
}




int fsync(int fildes)
{
    FIL *fh;
    FRESULT res;

    errno = 0;

    if(isatty(fildes))
    {
        errno = EBADF;
        return(-1);
    }


    // fileno_to_fatfs checks for fildes out of bounds
    fh = fileno_to_fatfs(fildes);
    if(fh == NULL)
    {
        errno = EBADF;
        return(-1);
    }

    res  = f_sync ( fh );
    if (res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(-1);
    }
    return(0);
}





int ftruncate(int fildes, off_t length)
{
    errno = 0;
    FIL *fh;
    FRESULT rc;

    if(isatty(fildes))
        return(-1);

    // fileno_to_fatfs checks for fildes out of bounds
    fh = fileno_to_fatfs(fildes);
    if(fh == NULL)
    {
        errno = EBADF;
        return(-1);
    }

    rc = f_lseek(fh, length);
    if (rc != FR_OK)
    {
        errno = fatfs_to_errno(rc);
        return(-1);
    }

    rc = f_truncate(fh);
    if (rc != FR_OK)
    {
        errno = fatfs_to_errno(rc);
        return(-1);
    }
    return(0);

}




/// @brief POSIX truncate named file to length.
///
/// - man page truncate (2).
///
/// @param[in] path: file name to truncate.
/// @param[in] length: length to truncate to.
///
/// @return 0 on sucess.
/// @return -1 n fail.
int truncate(const char *path, off_t length)
{
    errno = 0;
    FIL fh;
    FRESULT rc;

    rc = f_open(&fh , path, FA_OPEN_EXISTING | FA_READ | FA_WRITE);
    if (rc != FR_OK)
    {
        errno = fatfs_to_errno(rc);
        return(-1);
    }
    rc = f_lseek(&fh, length);
    if (rc != FR_OK)
    {
        errno = fatfs_to_errno(rc);
        return(-1);
    }
    rc = f_truncate(&fh);
    if (rc != FR_OK)
    {
        errno = fatfs_to_errno(rc);
        return(-1);
    }
    return(0);
}


int fat_close(int fildes)
{
    FIL *fh;
    int res;

    errno = 0;

    if(isatty(fildes))
        return(-1);

    // fileno_to_fatfs checks for fildes out of bounds
    fh = fileno_to_fatfs(fildes);
    if(fh == NULL)
    {
        errno = EBADF;
        return(-1);
    }

    res = f_close(fh);
    free_file_descriptor(fildes);
    if (res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(-1);
    }
    return(0);


}

int fstat(int fildes, struct stat *st)
{
    FIL *fh;

    // fileno_to_fatfs checks for fildes out of bounds
    fh = fileno_to_fatfs(fildes);
    if(fh == NULL)
    {
        errno = EBADF;
        return(-1);
    }

    st->st_mode = isatty(fildes) ? S_IFCHR : S_IFREG;
    st->st_size = f_size(fh);
    return 0;
}

int stat(const char *path, struct stat *st)
{
	errno = 0;
    FILINFO f;
    memset(&f, 0, sizeof(f));

    FRESULT res = f_stat(path, &f);

    if (res != FR_OK) {
        errno = fatfs_to_errno(res);
        return(-1);
    }

    st->st_size = f.fsize;
    st->st_mode = 0;
    st->st_mode |= (f.fattrib & AM_DIR) ? S_IFDIR : S_IFREG;
    st->st_mode |= (f.fattrib & AM_RDO) ?
                   (S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) :
                   (S_IRWXU | S_IRWXG | S_IRWXO);

    return 0;
}

// =============================================
// =============================================
///  - POSIX file and directory manipulation
// =============================================
// =============================================
/// @brief POSIX change directory.
///
/// - man page chdir (2).
///
/// @param[in] pathname: directory to change to
///
/// @return 0 on sucess.
/// @return -1 on error with errno set.
int chdir(const char *pathname)
{
    errno = 0;

    int res = f_chdir(pathname);
    if(res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(-1);
    }
    return(0);
}

/// @brief POSIX chmod function - change file access permission
/// Unfortunately file f_open modes and f_chmod modes are not the same
/// Files that are open have way more options - but only while the file is open.
///  - so this is a bit of a hack - we can only set read only  - if on one has write perms
///
/// - man page chmod (2).
///
/// @param[in] pathname: filename string.
/// @param[in] mode: POSIX chmod modes.
///
/// @return fileno on success.
int chmod(const char *pathname, mode_t mode)
{
    int rc;
    errno = 0;

    // FIXME for now we combine user,group and other perms and ask if anyone has write perms ?

    // Read only ???
    if ( !( mode & ( S_IWUSR | S_IWGRP | S_IWOTH)))
    {
        // file is read only
        rc = f_chmod(pathname, AM_RDO, AM_RDO);
        if (rc != FR_OK)
        {
            errno = fatfs_to_errno(rc);
            return(-1);
        }
    }

    return(0);
}

/// @brief POSIX get current working directory
///
/// - man page getcwd (2).
///
/// @param[in] pathname: directory to change to
///
/// @return 0 on sucess.
/// @return -1 on error with errno set.
char *getcwd(char *pathname, size_t len)
{
    int res;
    errno = 0;

    res = f_getcwd(pathname, (UINT)len);
    if(res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(NULL);
    }
    return(pathname);
}

/// @brief POSIX make a directory.
///
/// - man page mkdir (2).
///
/// @param[in] pathname: directory to create
///
/// @return 0 on sucess.
/// @return -1 on error with errno set.
int mkdir(const char *pathname, mode_t mode)
{
    errno = 0;

    if(mode)
    {
        if(chmod(pathname, mode))
            return(-1);
    }

    int res = f_mkdir(pathname);
    if(res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(-1);
    }
    return(0);
}


/// @brief POSIX rename a file by name.
///
/// - man page (2).
///
/// @param[in] oldpath: original name.
/// @param[in] newpath: new name.
///
/// @return 0 on success.
/// @return -1 on error with errno set.
int rename(const char *oldpath, const char *newpath)
{
/* Rename an object */
    int rc;
    errno = 0;
    rc = f_rename(oldpath, newpath);
    if(rc)
    {
        errno = fatfs_to_errno(rc);
        return(-1);
    }
    return(0);
}

/// @brief POSIX delete a directory.
///
/// - man page rmdir (2).
///
/// @param[in] pathname: directory to delete.
///
/// @return 0 on sucess.
/// @return -1 on error with errno set.
int rmdir(const char *pathname)
{
    errno = 0;
    int res = f_unlink(pathname);
    if(res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(-1);
    }
    return(0);
}

/// @brief POSIX delete a file.
///
/// - man page unlink (2).
///
/// @param[in] pathname: filename to delete.
///
/// @return 0 on sucess.
/// @return -1 on error with errno set.
int unlink(const char *pathname)
{
    errno = 0;
    int res = f_unlink(pathname);
    if(res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(-1);
    }
    return(0);
}


// =============================================
// =============================================
///   - POSIX - directory scanning functions
// =============================================
// =============================================
/// @brief POSIX closedir
/// - man page closedir (2).
///
/// @param[in] dirp: DIR * directory handle
///
/// @return 0 on sucess.
/// @return -1 on error with errno set.
int closedir(DIR *dirp)
{
    int res = f_closedir (dirp);
    if(res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(-1);
    }
    return(0);
}

/// @brief POSIX opendir
/// - man page opendir (2).
///
/// @param[in] pathname: directory to delete.
///
/// @return DIR * on sucess.
/// @return NULL on error with errno set.
static DIR _dp;
DIR *opendir(const char *pathdir)
{
    int res = f_opendir((DIR *) &_dp, pathdir);
    if(res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(NULL);
    }
    return ((DIR *) &_dp);
}

/// @brief POSIX opendir
/// - man page readdir(2).
///
/// @param[in] dirp: DIR * directory handle
///
/// @return DIR * on sucess.
/// @return NULL on error with errno set.
static  dirent_t _de;
dirent_t * readdir(DIR *dirp)
{
    FILINFO fno;
    int len;
    int res;

    _de.d_name[0] = 0;
    res = f_readdir ( dirp, &fno );
    if(res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(NULL);
    }
    _de.d_type = (fno.fattrib & AM_DIR) ? DT_DIR : DT_REG;

    len = strlen(fno.fname);
    strncpy(_de.d_name,fno.fname,len);
    _de.d_name[len] = 0;
    return( (dirent_t *) &_de);
}

