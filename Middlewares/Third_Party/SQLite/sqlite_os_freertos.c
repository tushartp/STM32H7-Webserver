/*
 * sqlite_os_freertos.c
 *
 *  Created on: Oct 31, 2017
 *      Author: tcpatel
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include "sqliteInt.h"
//#include "sqlite3.h"
#include "FreeRTOS.h"
#include "os_common.h"
#include "fatfs_wrapper.h"

//uint32_t xpBufSqlite_Heap[20*1024];
extern int os_get_random(unsigned char *buf, size_t len);

//#define OSTRACE configPRINTF
/*
** Size of the write buffer used by journal files in bytes.
*/
#ifndef SQLITE_DEMOVFS_BUFFERSZ
# define SQLITE_DEMOVFS_BUFFERSZ 4096
#endif


int sqlite_config_options();

#ifdef SQLITE_FREERTOS_MALLOC
/*
 * Memory Implementation for the Sqlite
 */
/*
** Allocate Bytes of memory.
*/
void *xSqliteMemMalloc(int Bytes)
{
	return pvPortMalloc((size_t)Bytes);
}
/*
 * Free Memory
 */
void vSqliteMemFree(void *pv)
{
	vPortFree(pv);
}

/*
 * Change the size of existing memory allocation
 */
void *xSqliteMemRealloc(void *pv, int Bytes)
{
	return pvPortRealloc(pv, Bytes);
}

/*
 * return the size of an outstanding allocation in bytes
 */
int SqliteMemSize(void *pv)
{
	return (int) xPortGetMemSize(pv);
}

/*
* Round up a request size to the next valid allocation size.
*/
static int pvSqliteMemRoundup(int n){
    return n;
}

/*
 * Initialize the Mem Unit
 */
static int pvSqliteMemInit(void *pAppData)
{
	UNUSED_PARAMETER(pAppData);
	return (int) SQLITE_OK;
}
/*
 * Shutdown the module
 */
static void pvSqliteMemShutdown(void *pAppData)
{
	UNUSED_PARAMETER(pAppData);
}
/*
** An instance of this object defines the interface between SQLite
** and low-level memory allocation routines.
**
** This routine is only called by sqlite3_config(), and therefore
** is not required to be threadsafe (it is not).
*/
const sqlite3_mem_methods *sqlite3MemGetFreeRTOS(void){
    static const sqlite3_mem_methods MemMethods = {
    		xSqliteMemMalloc,
			vSqliteMemFree,
			xSqliteMemRealloc,
			SqliteMemSize,
			pvSqliteMemRoundup,
			pvSqliteMemInit,
			pvSqliteMemShutdown,
			NULL
    };
    return &MemMethods;
}

void sqlite3MemSetDefault(void){
    sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetFreeRTOS());
}

#endif
/*
 * VFS layer Implementation for Sqlite with Fatfs
 *
 *
 *
 */

/*******************************************************************************
* Type definitions, macros, and constants                                      *
*******************************************************************************/
#define MAXPATHNAME  ( 256 )

#ifndef MAXDWORD
#define MAXDWORD     ( 0xFFFFFFFF )   /* 4294967295 */
#endif


/*
** Allowed values for ctrlFlags
*/
#define FATFSFILE_RDONLY          0x02   /* Connection is read only */
#define FATFSFILE_PERSIST_WAL     0x04   /* Persistent WAL mode */
#define FATFSFILE_PSOW            0x10   /* SQLITE_IOCAP_POWERSAFE_OVERWRITE */

/*
** The rawFile structure is a subclass of sqlite3_file* specific to the RAW-OS
** portability layer.
*/
typedef struct FatFsFile FatFsFile;
struct FatFsFile {
  sqlite3_file base;					/* Base class. Must be first. */
  sqlite3_vfs *pVfs;                  /* The VFS that created this rawFile */
  FILE * h;                            	/* Pointer to access the file */
  DWORD sectorSize;                   /* Sector size of the device file is on */
  char name[MAXPATHNAME];
  unsigned short int ctrlFlags;       /* Behavioral bits.  RAWFILE_* flags */
  int szChunk;                        /* Configured by FCNTL_CHUNK_SIZE */
  char *aBuffer;                  		/* Pointer to malloc'd buffer */
  int nBuffer;                    		/* Valid bytes of data in zBuffer */
  sqlite3_int64 iBufferOfst;      /* Offset in file of zBuffer[0] */
};


/*******************************************************************************
* Static Function Prototypes                                                   *
*******************************************************************************/
static int xFreeRTOSDirectWrite(FatFsFile *p, const void *zBuf, int iAmt, sqlite_int64 iOfst);
static int xFreeRTOSFlushBuffer(FatFsFile *p);
static int xFreeRTOSClose(sqlite3_file *id);
static int xFreeRTOSRead(sqlite3_file *id, void *pBuf, int amt, sqlite3_int64 offset);
static int xFreeRTOSWrite(sqlite3_file *id, const void *pBuf, int amt, sqlite3_int64 offset);
static int xFreeRTOSTruncate(sqlite3_file *id, sqlite3_int64 nByte);
static int xFreeRTOSSync(sqlite3_file *id, int flags);
static int xFreeRTOSFileSize(sqlite3_file *id, sqlite3_int64 *pSize);
static int xFreeRTOSLock(sqlite3_file *id, int eLock);
static int xFreeRTOSUnlock(sqlite3_file *id, int eLock);
static int xFreeRTOSCheckReservedLock(sqlite3_file *id, int *pResOut);
static int xFreeRTOSFileControl(sqlite3_file *id, int op, void *pArg);
static int xFreeRTOSSectorSize(sqlite3_file *id);
static int xFreeRTOSDeviceChar(sqlite3_file *id);
static int xFreeRTOSAccess(sqlite3_vfs *pVfs, const char *zFilename, int flags, int *pResOut);
/*******************************************************************************
* Local Functions                                                              *
*******************************************************************************/
#if defined(SQLITE_DEBUG_OS_TRACE)
static const char * fatfsErrName(FRESULT rc)
{
  static char const * name[] = {
    "OK",
    "DISK_ERR",
    "INT_ERR",
    "NOT_READY",
    "NO_FILE",
    "NO_PATH",
    "INVALID_NAME",
    "DENIED",
    "EXIST",
    "INVALID_OBJECT",
    "WRITE_PROTECTED",
    "INVALID_DRIVE",
    "NOT_ENABLED",
    "NO_FILE_SYSTEM",
    "MKFS_ABORTED",
    "TIMEOUT",
    "LOCKED",
    "NOT_ENOUGH_CORE",
    "TOO_MANY_OPEN_FILES"
  };

  if(rc >= (sizeof(name)/sizeof(name[0])))
  {
    return NULL;
  }

  return name[rc];
}
#endif

/*
** Create a temporary file name in zBuf.  zBuf must be big enough to
** hold at pVfs->mxPathname characters.
*/
static void prvGetTempPath(int len, char *path)
{
    if (len > 2)
    {
        strcpy(path, "0:");
    }
}

static int prvGetTempname(int nBuf, char *zBuf){
  static char zChars[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789";
  size_t i, j;
  char zTempPath[MAXPATHNAME+1];
  if( sqlite3_temp_directory ){
    sqlite3_snprintf(MAXPATHNAME-30, zTempPath, "%s", sqlite3_temp_directory);
  }else {
    char *zMulti;
    char zPath[MAXPATHNAME];
    prvGetTempPath(MAXPATHNAME-30, zPath);
    zMulti = zPath;
    if( zMulti ){
      sqlite3_snprintf(MAXPATHNAME-30, zTempPath, "%s", zMulti);
    }else{
      return SQLITE_NOMEM;
    }
  }
  for(i=sqlite3Strlen30(zTempPath); i>0 && zTempPath[i-1]=='/'; i--){}
  zTempPath[i] = 0;
  sqlite3_snprintf(nBuf-30, zBuf,
                   "%s/"SQLITE_TEMP_FILE_PREFIX, zTempPath);
  j = sqlite3Strlen30(zBuf);
  sqlite3_randomness(20, &zBuf[j]);
  for(i=0; i<20; i++, j++){
    zBuf[j] = (char)zChars[ ((unsigned char)zBuf[j])%(sizeof(zChars)-1) ];
  }
  zBuf[j] = 0;
  configPRINTF("TEMP FILENAME: %s\n", zBuf);
  return SQLITE_OK;
}


/*
** Write directly to the file passed as the first argument. Even if the
** file has a write-buffer (DemoFile.aBuffer), ignore it.
*/
static int xFreeRTOSDirectWrite(
  FatFsFile *p,                    /* File handle */
  const void *zBuf,               /* Buffer containing data to write */
  int iAmt,                       /* Size of data to write in bytes */
  sqlite_int64 iOfst              /* File offset to write to */
){
  off_t ofst;                     /* Return value from lseek() */
  size_t nWrite;                  /* Return value from write() */

  ofst = fseek(p->h, iOfst, SEEK_SET);
  if( ofst!= 0 ){
    return SQLITE_IOERR_WRITE;
  }

  nWrite = fwrite( zBuf, 1, iAmt, p->h);
  if( nWrite!=iAmt ){
	configPRINTF("Flush file write=%p, name=%s\n", p->h, p->name);
    return SQLITE_IOERR_WRITE;
  }

  return SQLITE_OK;
}

/*
** Flush the contents of the DemoFile.aBuffer buffer to disk. This is a
** no-op if this particular file does not have a buffer (i.e. it is not
** a journal file) or if the buffer is currently empty.
*/
static int xFreeRTOSFlushBuffer(FatFsFile *p){
  int rc = SQLITE_OK;
  if( p->nBuffer ){
    rc = xFreeRTOSDirectWrite(p, p->aBuffer, p->nBuffer, p->iBufferOfst);
    p->nBuffer = 0;
  }
  return rc;
}


/*****************************************************************************
** The next group of routines implement the I/O methods specified
** by the sqlite3_io_methods object.
******************************************************************************/

/*
** Close a file.
*/
static int xFreeRTOSClose(sqlite3_file *id){
	  int rc;
	  FatFsFile *pFile = (FatFsFile*)id;

	  assert( id!=0 );
	  assert( pFile->h!=NULL );

	  rc = xFreeRTOSFlushBuffer(pFile);
	  sqlite3_free(pFile->aBuffer);

	  rc = fclose(pFile->h);

	  if( rc == 0 ){
	    sqlite3_free(pFile->h);
	    pFile->h = NULL;
	  }

	  OpenCounter(-1);
	  configPRINTF("CLOSE file=%p, name=%s, rc=%d\n", pFile->h, pFile->name, errno);

	  return (rc == 0) ? SQLITE_OK : SQLITE_IOERR_CLOSE;

}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
static int xFreeRTOSRead(
  sqlite3_file *id,          /* File to read from */
  void *pBuf,                /* Write content into this buffer */
  int amt,                   /* Number of bytes to read */
  sqlite3_int64 offset       /* Begin reading at this offset */
){
  int rc;
  FatFsFile *pFile = (FatFsFile*)id;

  assert( id!=0 );
  assert( amt>0 );
  assert( offset>=0 && offset <= MAXDWORD);
  SimulateIOError(return SQLITE_IOERR_READ);
  configPRINTF("1:READ file=%p, buffer=%p, amount=%d, offset=%lld\n", pFile->h, pBuf, amt, offset);

  /* Flush any data in the write buffer to disk in case this operation
  ** is trying to read data the file-region currently cached in the buffer.
  ** It would be possible to detect this case and possibly save an
  ** unnecessary write here, but in practice SQLite will rarely read from
  ** a journal file when there is data cached in the write-buffer.
  */
  rc = xFreeRTOSFlushBuffer(pFile);
  if( rc!=SQLITE_OK ){
    return rc;
  }

  /* seek pos */
  rc = fseek(pFile->h, (DWORD)offset, SEEK_SET);
  if( rc != 0 ){
	configPRINTF("2:READ file=%p, rc=%d\n", pFile->h, errno);
	return SQLITE_IOERR_SEEK;
  }

  /* do read file */
  rc = fread( pBuf, amt, 1, pFile->h);

  if(rc == amt)
  {
	  configPRINTF("3:READ file=%p, rc=SQLITE_OK\n", pFile->h);
	  return SQLITE_OK;
  }
  else if(rc >=0)
  {
	  if(rc<amt)
	  {
	/* Unread parts of the buffer must be zero-filled */
		memset(&((char*)pBuf)[rc], 0, amt-rc);
		configPRINTF("4:READ file=%p, rc=SQLITE_IOERR_SHORT_READ\n", pFile->h);
	return SQLITE_IOERR_SHORT_READ;
  }
  }
  configPRINTF("5:READ FAIL %s\n", pFile->name);
  return SQLITE_IOERR_READ;
}

/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
static int xFreeRTOSWrite(
  sqlite3_file *id,         /* File to write into */
  const void *pBuf,         /* The bytes to be written */
  int amt,                  /* Number of bytes to write */
  sqlite3_int64 offset      /* Offset into the file to begin writing at */
){
  FatFsFile *pFile = (FatFsFile*)id;
  assert( amt>0 );
  assert( pFile );
  assert( offset>=0 && offset <= MAXDWORD);

  configPRINTF("1:WRITE name=%s file=%p, buffer=%p, amount=%d, offset=%lld\n", pFile->name, pFile->h, pBuf, amt, offset);



  if( pFile->aBuffer ){
    char *z = (char *)pBuf;       /* Pointer to remaining data to write */
    int n = amt;                 /* Number of bytes at z */
    sqlite3_int64 i = offset;      /* File offset to write to */

    while( n>0 ){
      int nCopy;                  /* Number of bytes to copy into buffer */

      /* If the buffer is full, or if this data is not being written directly
      ** following the data already buffered, flush the buffer. Flushing
      ** the buffer is a no-op if it is empty.
      */
      if( pFile->nBuffer==SQLITE_DEMOVFS_BUFFERSZ || pFile->iBufferOfst+pFile->nBuffer!=i ){
        int rc = xFreeRTOSFlushBuffer(pFile);
        if( rc!=SQLITE_OK ){
          return rc;
        }
  }
      assert( pFile->nBuffer==0 || pFile->iBufferOfst + pFile->nBuffer==i );
      pFile->iBufferOfst = i - pFile->nBuffer;

      /* Copy as much data as possible into the buffer. */
      nCopy = SQLITE_DEMOVFS_BUFFERSZ - pFile->nBuffer;
      if( nCopy>n ){
        nCopy = n;
  }
      memcpy(&pFile->aBuffer[pFile->nBuffer], z, nCopy);
      pFile->nBuffer += nCopy;

      n -= nCopy;
      i += nCopy;
      z += nCopy;
    }
  }else{
    return xFreeRTOSDirectWrite(pFile, pBuf, amt, offset);
  }

  configPRINTF("WRITE file=%p, rc=SQLITE_OK\n", pFile->h);
  return SQLITE_OK;
}

/*
** Truncate an open file to a specified size
*/
static int xFreeRTOSTruncate(sqlite3_file *id, sqlite3_int64 nByte){
  FatFsFile *pFile = (FatFsFile*)id;

  configPRINTF("TRUNCATE file\n");

  return 0 ? SQLITE_IOERR_TRUNCATE :SQLITE_OK;
}

/*
** Make sure all writes to a particular file are committed to disk.
*/
static int xFreeRTOSSync(sqlite3_file *id, int flags){
  FRESULT rc;
  FatFsFile *pFile = (FatFsFile*)id;

  assert( id!=0 );
  UNUSED_PARAMETER(flags);
  configPRINTF("SYNC %d\n", pFile->h);

  rc = xFreeRTOSFlushBuffer(pFile);
  if( rc!=SQLITE_OK ){
    return rc;
  }

  rc = fsync((pFile->h)->_file);

  if(rc == 0){
    configPRINTF("SYNC %d SQLITE_OK\n", pFile->h);
    return SQLITE_OK;
  }else{
    configPRINTF("SYNC %d rc=%d\n", pFile->h, errno);
    return SQLITE_IOERR;
  }
}

/*
** Determine the current size of a file in bytes
*/
static int xFreeRTOSFileSize(sqlite3_file *id, sqlite3_int64 *pSize){

	FatFsFile *pFile = (FatFsFile*)id;
  int rc;
  struct stat sStat;              /* Output of fstat() call */
  assert( id!=0 );
  assert( pSize!=0 );
  configPRINTF("SIZE file=%p, pSize=%p\n", pFile->h, pSize);

  rc = xFreeRTOSFlushBuffer(pFile);
  if( rc!=SQLITE_OK ){
    return rc;
  }

  rc = fstat((pFile->h)->_file, &sStat);

  if( rc!=0 ) return SQLITE_IOERR_FSTAT;

  *pSize = sStat.st_size;

  configPRINTF("SIZE file=%p, pSize=%p, *pSize=%lld\n",
           pFile->h, pSize, *pSize);

  return SQLITE_OK;
}

/*
** Locking functions. The xLock() and xUnlock() methods are both no-ops.
** The xCheckReservedLock() always indicates that no other process holds
** a reserved lock on the database file. This ensures that if a hot-journal
** file is found in the file-system it is rolled back.
*/

static int xFreeRTOSLock(sqlite3_file *id, int eLock){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(eLock);
  return SQLITE_OK;
}

static int xFreeRTOSUnlock(sqlite3_file *id, int eLock){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(eLock);
  return SQLITE_OK;
}
/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, return
** non-zero, otherwise zero.
*/
static int xFreeRTOSCheckReservedLock(sqlite3_file *id, int *pResOut){
  UNUSED_PARAMETER(id);
  *pResOut = 0;
  return SQLITE_OK;
}

/*
** If *pArg is inititially negative then this is a query.  Set *pArg to
** 1 or 0 depending on whether or not bit mask of pFile->ctrlFlags is set.
**
** If *pArg is 0 or 1, then clear or set the mask bit of pFile->ctrlFlags.
*/
static void pvFreeRTOSModeBit(FatFsFile *pFile, unsigned char mask, int *pArg){
  if( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & mask)!=0;
  }else if( (*pArg)==0 ){
    pFile->ctrlFlags &= ~mask;
  }else{
    pFile->ctrlFlags |= mask;
  }
}


/*
** Control and query of the open file handle.
*/

static int xFreeRTOSFileControl(sqlite3_file *id, int op, void *pArg){
	FatFsFile *pFile = (FatFsFile*)id;
	  configPRINTF("FCNTL file=%p, op=%d, pArg=%p\n", pFile->h, op, pArg);
	  switch( op ){
	    case SQLITE_FCNTL_CHUNK_SIZE: {
	      pFile->szChunk = *(int *)pArg;
	      configPRINTF("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h);
	      return SQLITE_OK;
	    }
	    case SQLITE_FCNTL_SIZE_HINT: {
	      if( pFile->szChunk>0 ){
	        sqlite3_int64 oldSz;
	        int rc = xFreeRTOSFileSize(id, &oldSz);
	        if( rc==SQLITE_OK ){
	          sqlite3_int64 newSz = *(sqlite3_int64*)pArg;
	          if( newSz>oldSz ){
	            SimulateIOErrorBenign(1);
	            rc = xFreeRTOSTruncate(id, newSz);
	            SimulateIOErrorBenign(0);
	          }
	        }
	        configPRINTF("FCNTL file=%p\n", pFile->h);
	        return rc;
	      }
	      configPRINTF("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h);
	      return SQLITE_OK;
	    }
	    case SQLITE_FCNTL_PERSIST_WAL: {
	    	pvFreeRTOSModeBit(pFile, FATFSFILE_PERSIST_WAL, (int*)pArg);
	      configPRINTF("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h);
	      return SQLITE_OK;
	    }
	    case SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
	    	pvFreeRTOSModeBit(pFile, FATFSFILE_PSOW, (int*)pArg);
	      configPRINTF("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h);
	      return SQLITE_OK;
	    }
	    case SQLITE_FCNTL_VFSNAME: {
	      *(char**)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
	      configPRINTF("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h);
	      return SQLITE_OK;
	    }
	    case SQLITE_FCNTL_TEMPFILENAME: {
	      char *zTFile = sqlite3_malloc( pFile->pVfs->mxPathname );
	      if( zTFile ){
	    	  prvGetTempname(pFile->pVfs->mxPathname, zTFile);
	        *(char**)pArg = zTFile;
	      }
	      return SQLITE_OK;
	    }
	  }
	  configPRINTF("FCNTL file=%p, rc=SQLITE_NOTFOUND\n", pFile->h);
	  return SQLITE_NOTFOUND;
}
/*
** Return the sector size in bytes of the underlying block device for
** the specified file. This is almost always 512 bytes, but may be
** larger for some devices.
**
** SQLite code assumes this function cannot fail. It also assumes that
** if two files are created in the same file-system directory (i.e.
** a database and it's journal file) that the sector size will be the
** same for both.
** Sector size if fixed to 512 because of setting size to 512 in FatsFs
*/
static int xFreeRTOSSectorSize(sqlite3_file *id){

    /* To avoid unused parameter compiler warning */
	UNUSED_PARAMETER(id);

    return SQLITE_DEFAULT_SECTOR_SIZE;
}

/*
** Return a vector of device characteristics.
*/
static int xFreeRTOSDeviceChar(sqlite3_file *id){
	  FatFsFile *p = (FatFsFile*)id;
	  assert( id!=0 );
	  return SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN |
	         ((p->ctrlFlags & FATFSFILE_PSOW) ? SQLITE_IOCAP_POWERSAFE_OVERWRITE : 0);
}


/*
** If possible, return a pointer to a mapping of file fd starting at offset
** iOff. The mapping must be valid for at least nAmt bytes.
**
** If such a pointer can be obtained, store it in *pp and return SQLITE_OK.
** Or, if one cannot but no error occurs, set *pp to 0 and return SQLITE_OK.
** Finally, if an error does occur, return an SQLite error code. The final
** value of *pp is undefined in this case.
**
** If this function does return a pointer, the caller must eventually
** release the reference by calling unixUnfetch().
*/
static int xFreeRTOSFetch(sqlite3_file* fd, sqlite3_int64 iOff, int nAmt, void **pp){
#if SQLITE_MAX_MMAP_SIZE>0
	#error define some logic over here
#else
	  UNUSED_PARAMETER(fd);
	  UNUSED_PARAMETER(iOff);
	  UNUSED_PARAMETER(nAmt);
	  *pp=0;
#endif
	  return SQLITE_OK;
}


/*
** If the third argument is non-NULL, then this function releases a
** reference obtained by an earlier call to unixFetch(). The second
** argument passed to this function must be the same as the corresponding
** argument that was passed to the unixFetch() invocation.
**
** Or, if the third argument is NULL, then this function is being called
** to inform the VFS layer that, according to POSIX, any existing mapping
** may now be invalid and should be unmapped.
*/
static int xFreeRTOSUnFetch(sqlite3_file* fd, sqlite3_int64 iOff, void *p){
#if SQLITE_MAX_MMAP_SIZE>0
	#error define some logic over here
#else
	  UNUSED_PARAMETER(fd);
	  UNUSED_PARAMETER(p);
	  UNUSED_PARAMETER(iOff);
#endif
	  return SQLITE_OK;
}
/*
** This vector defines all the methods that can operate on an OsFile
** for nucleus.
*/
static const sqlite3_io_methods sqlite3FreeRTOSIoMethod = {
		  3,							/*int iVersion;*/
		  xFreeRTOSClose,				/*int (*xClose)(sqlite3_file*);*/
		  xFreeRTOSRead,				/*int (*xRead)(sqlite3_file*, void*, int iAmt, sqlite3_int64 iOfst);*/
		  xFreeRTOSWrite,				/*int (*xWrite)(sqlite3_file*, const void*, int iAmt, sqlite3_int64 iOfst);*/
		  xFreeRTOSTruncate,			/*int (*xTruncate)(sqlite3_file*, sqlite3_int64 size);*/
		  xFreeRTOSSync,				/*int (*xSync)(sqlite3_file*, int flags);*/
		  xFreeRTOSFileSize,			/*int (*xFileSize)(sqlite3_file*, sqlite3_int64 *pSize);*/
		  xFreeRTOSLock,				/*int (*xLock)(sqlite3_file*, int);*/
		  xFreeRTOSUnlock,				/*int (*xUnlock)(sqlite3_file*, int);*/
		  xFreeRTOSCheckReservedLock,	/*int (*xCheckReservedLock)(sqlite3_file*, int *pResOut);*/
		  xFreeRTOSFileControl,			/*int (*xFileControl)(sqlite3_file*, int op, void *pArg);*/
		  xFreeRTOSSectorSize,			/*int (*xSectorSize)(sqlite3_file*);*/
		  xFreeRTOSDeviceChar,			/*int (*xDeviceCharacteristics)(sqlite3_file*);*/
		  /* Methods above are valid for version 1 */
		  NULL,							/*int (*xShmMap)(sqlite3_file*, int iPg, int pgsz, int, void volatile**);*/
		  NULL,							/*int (*xShmLock)(sqlite3_file*, int offset, int n, int flags);*/
		  NULL,							/*void (*xShmBarrier)(sqlite3_file*);*/
		  NULL,							/*int (*xShmUnmap)(sqlite3_file*, int deleteFlag);*/
		  /* Methods above are valid for version 2 */
		  xFreeRTOSFetch,				/*int (*xFetch)(sqlite3_file*, sqlite3_int64 iOfst, int iAmt, void **pp);*/
		  xFreeRTOSUnFetch,				/*int (*xUnfetch)(sqlite3_file*, sqlite3_int64 iOfst, void *p);*/
		  /* Methods above are valid for version 3 */
		  /* Additional methods may be added in future releases */
};


/***************************************************************************
** Here ends the I/O methods that form the sqlite3_io_methods object.
**
** The next block of code implements the VFS methods.
****************************************************************************/
/*
** Open a file.
*/
static int xFreeRTOSOpen(
  sqlite3_vfs *pVfs,        /* The VFS for which this is the xOpen method */
  const char *zName,        /* Pathname of file to be opened (UTF-8) */
  sqlite3_file *id,         /* Write the SQLite file handle here */
  int flags,                /* Open mode flags */
  int *pOutFlags            /* Status return flags */
){
  FatFsFile *pFile = (FatFsFile *)id;
  FILE *h = NULL;
  char *mode = "r";
  char *aBuf = 0;

  if (zName == NULL)
	  return SQLITE_IOERR;

  if( flags&SQLITE_OPEN_READONLY )  mode = "r";

  if( flags&SQLITE_OPEN_READWRITE || flags&SQLITE_OPEN_MAIN_JOURNAL ) {
	  int result;
	  if (SQLITE_OK != xFreeRTOSAccess(pVfs, zName, flags, &result))
		  return SQLITE_CANTOPEN;

	  if (result == 1)
		  mode = "r+";
	  else
		  mode = "w+";
  }

  configPRINTF("OPEN: %s %s\n", zName, mode);
  memset (pFile, 0, sizeof(FatFsFile));

  strncpy (pFile->name, zName, MAXPATHNAME);
  pFile->name[MAXPATHNAME-1] = '\0';


  if( flags&SQLITE_OPEN_MAIN_JOURNAL ){
    aBuf = (char *)sqlite3_malloc(SQLITE_DEMOVFS_BUFFERSZ);
    if( !aBuf ){
      return SQLITE_NOMEM;
    }
  }
  h = fopen(zName, mode);
  if(h == NULL)
  {
	  return SQLITE_CANTOPEN;
  }
  configPRINTF("OPEN name=%s, mode=%s\n", zName, mode);
  pFile->base.pMethods = &sqlite3FreeRTOSIoMethod;

  return SQLITE_OK;
}



#if 0
/*
** Open a file.
*/
static int xFreeRTOSOpen(
  sqlite3_vfs *pVfs,        /* The VFS for which this is the xOpen method */
  const char *zName,        /* Pathname of file to be opened (UTF-8) */
  sqlite3_file *id,         /* Write the SQLite file handle here */
  int flags,                /* Open mode flags */
  int *pOutFlags            /* Status return flags */
){
  FatFsFile *pFile = (FatFsFile *)id;
  FIL *h;
  int openFlags = 0;             /* Flags to pass to open() */
  int eType = flags&0xFFFFFF00;  /* Type of file to open */
  int rc = SQLITE_OK;            /* Function Return Code */

  int isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  int isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  int isCreate     = (flags & SQLITE_OPEN_CREATE);
  int isReadonly   = (flags & SQLITE_OPEN_READONLY);
  int isReadWrite  = (flags & SQLITE_OPEN_READWRITE);

  /* If creating a master or main-file journal, this function will open
  ** a file-descriptor on the directory too. The first time rawSync()
  ** is called the directory file descriptor will be fsync()ed and close()d.
  */
  int syncDir = (isCreate && (
        eType==SQLITE_OPEN_MASTER_JOURNAL
     || eType==SQLITE_OPEN_MAIN_JOURNAL
     || eType==SQLITE_OPEN_WAL
  ));

  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  char zTmpname[MAXPATHNAME+2];

  /* Check the following statements are true:
  **
  **   (a) Exactly one of the READWRITE and READONLY flags must be set, and
  **   (b) if CREATE is set, then READWRITE must also be set, and
  **   (c) if EXCLUSIVE is set, then CREATE must also be set.
  **   (d) if DELETEONCLOSE is set, then CREATE must also be set.
  */
  assert((isReadonly==0 || isReadWrite==0) && (isReadWrite || isReadonly));
  assert(isCreate==0 || isReadWrite);
  assert(isExclusive==0 || isCreate);
  assert(isDelete==0 || isCreate);

  /* The main DB, main journal, WAL file and master journal are never
  ** automatically deleted. Nor are they ever temporary files.  */
  assert( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_DB );
  assert( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_JOURNAL );
  assert( (!isDelete && zName) || eType!=SQLITE_OPEN_MASTER_JOURNAL );
  assert( (!isDelete && zName) || eType!=SQLITE_OPEN_WAL );

  /* Assert that the upper layer has set one of the "file-type" flags. */
  assert( eType==SQLITE_OPEN_MAIN_DB      || eType==SQLITE_OPEN_TEMP_DB
       || eType==SQLITE_OPEN_MAIN_JOURNAL || eType==SQLITE_OPEN_TEMP_JOURNAL
       || eType==SQLITE_OPEN_SUBJOURNAL   || eType==SQLITE_OPEN_MASTER_JOURNAL
       || eType==SQLITE_OPEN_TRANSIENT_DB || eType==SQLITE_OPEN_WAL
  );


  if( !zName ){
    /* If zName is NULL, the upper layer is requesting a temp file. */
    assert(isDelete && !syncDir);
    rc = prvGetTempname(MAXPATHNAME+2, zTmpname);
    if( rc!=SQLITE_OK ){
      configPRINTF(("OPEN name=%s, rc=SQLITE_IOERR\n", zName));
      return SQLITE_IOERR;
    }
    zName = zTmpname;

    /* Generated temporary filenames are always double-zero terminated
    ** for use by sqlite3_uri_parameter(). */
    assert( zName[strlen(zName)+1]==0 );
  }

  /* Determine the value of the flags parameter passed to POSIX function
  ** open(). These must be calculated even if open() is not called, as
  ** they may be stored as part of the file handle and used by the
  ** 'conch file' locking functions later on.  */
  if( isReadWrite ) {
    openFlags = FA_READ | FA_WRITE;
  } else {
    openFlags = FA_READ;
  }

  if( isExclusive ) {
    openFlags |= FA_CREATE_NEW;
  } else if( isCreate ) {
    openFlags |= FA_OPEN_ALWAYS;
  } else {
    openFlags |= FA_OPEN_EXISTING;
  }

  configPRINTF(("OPEN name=%s, pFile=%p, flags=%x, pOutFlags=%p\n",
           zName, id, flags, pOutFlags));

  h = sqlite3MallocZero(sizeof(FILE));
  if( !h ){
    configPRINTF(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM\n", zName));
    return SQLITE_IOERR_NOMEM;
  }

  rc = f_open(h, (TCHAR *)zName, openFlags);
  if( rc != FR_OK ){
    configPRINTF(("OPEN name=%s, rc=%s\n", zName, fatfsErrName(rc)));
    return SQLITE_CANTOPEN;
  }

  if( pOutFlags ){
    if( flags & SQLITE_OPEN_READWRITE ){
      *pOutFlags = SQLITE_OPEN_READWRITE;
    }else{
      *pOutFlags = SQLITE_OPEN_READONLY;
    }
  }

  configPRINTF(("OPEN name=%s, access=%lx, pOutFlags=%p, *pOutFlags=%d\n",
           zName, openFlags, pOutFlags, pOutFlags ? *pOutFlags : 0));

  memset(pFile, 0, sizeof(FatFsFile));
  pFile->pMethod = &sqlite3FreeRTOSIoMethod;
  pFile->pVfs = pVfs;
  pFile->h = h;
#if _MAX_SS != _MIN_SS
  pFile->sectorSize = h->obj->fs.ssize;
#else
  pFile->sectorSize = SQLITE_DEFAULT_SECTOR_SIZE;
#endif
  pFile->zPath = zName;
  if( isReadonly ){
    pFile->ctrlFlags |= FATFSFILE_RDONLY;
  }
  if( sqlite3_uri_boolean(zName, "psow", SQLITE_POWERSAFE_OVERWRITE) ){
    pFile->ctrlFlags |= FATFSFILE_PSOW;
  }

  OpenCounter(+1);

  return SQLITE_OK;
}
#endif
/*
** Delete the named file.
**
*/
static int xFreeRTOSDelete(
  sqlite3_vfs *pVfs,          /* Not used */
  const char *zFilename,      /* Name of file to delete (UTF-8) */
  int syncDir                 /* Not used */
){
  FRESULT rc;

  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(syncDir);

  SimulateIOError(return SQLITE_IOERR_DELETE);

  // Delete file
  rc = remove(zFilename);
  if(rc == 0) {
    configPRINTF("DELETE name=%s, SQLITE_OK\n", zFilename);
    return SQLITE_OK;
  }else{
    configPRINTF("DELETE name=%s, errno=%d SQLITE_IOERR_DELETE\n", zFilename, errno);
    return SQLITE_IOERR_DELETE;
  }
}

/*
** Test the existence of or access permissions of file zPath. The
** test performed depends on the value of flags:
**
**     SQLITE_ACCESS_EXISTS: Return 1 if the file exists
**     SQLITE_ACCESS_READWRITE: Return 1 if the file is read and writable.
**     SQLITE_ACCESS_READONLY: Return 1 if the file is readable.
**
** Otherwise return 0.
*/
static int xFreeRTOSAccess(
  sqlite3_vfs *pVfs,         /* Not used on */
  const char *zFilename,     /* Name of file to check (UTF-8) */
  int flags,                 /* Type of test to make on this file */
  int *pResOut               /* OUT: Result */
){
  FRESULT rc;
  struct stat fi;
  UNUSED_PARAMETER(pVfs);

  SimulateIOError( return SQLITE_IOERR_ACCESS; );
  configPRINTF("ACCESS name=%s, flags=%x, pResOut=%p\n", zFilename, flags, pResOut);

  assert( pResOut );

  rc = stat(zFilename, &fi);

  switch( flags ){
    case SQLITE_ACCESS_READ:
    case SQLITE_ACCESS_EXISTS:
    case SQLITE_ACCESS_READWRITE:
      *pResOut = ( rc == 0 ) ? 1 : 0;
      break;
    default:
      assert(!"Invalid flags argument");
  }

  configPRINTF("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           zFilename, pResOut, *pResOut);
  return SQLITE_OK;
}


/*
** Turn a relative pathname into a full pathname.  Write the full
** pathname into zOut[].  zOut[] will be at least pVfs->mxPathname
** bytes in size.
*/
static int xFreeRTOSFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  const char *zPath,            /* Possibly relative input path */
  int nOut,                     /* Size of output buffer in bytes */
  char *zOut                    /* Output nbuffer */
){
  SimulateIOError( return SQLITE_ERROR );

  assert( pVfs->mxPathname==MAXPATHNAME);
  UNUSED_PARAMETER(pVfs);

  zOut[nOut-1] = '\0';
  if( zPath[1]==':' ){
    sqlite3_snprintf(nOut, zOut, "%s", zPath);
  }else{
    int nCwd;
    if( getcwd(zOut, nOut-1)==0 ){
      return SQLITE_IOERR;
    }
    nCwd = (int)strlen(zOut);
    sqlite3_snprintf(nOut-nCwd, &zOut[nCwd], "/%s", zPath);
  }
  return SQLITE_OK;
}

/*
** The following four VFS methods:
**
**   xDlOpen
**   xDlError
**   xDlSym
**   xDlClose
**
** are supposed to implement the functionality needed by SQLite to load
** extensions compiled as shared objects. This simple VFS does not support
** this functionality, so the following functions are no-ops.
*/
static void *pxFreeRTOSDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(zPath);
  return 0;
}

static void xFreeRTOSDlError(sqlite3_vfs *pVfs, int nByte, char *zErrMsg){
  UNUSED_PARAMETER(pVfs);
  sqlite3_snprintf(nByte, zErrMsg, "Loadable extensions are not supported");
  zErrMsg[nByte-1] = '\0';
}

static void (*pxFreeRTOSDlSym(sqlite3_vfs *pVfs, void *pH, const char *z))(void){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(pH);
  UNUSED_PARAMETER(z);
  return 0;
}

static void xFreeRTOSDlClose(sqlite3_vfs *pVfs, void *pHandle){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(pHandle);
}

/*
** Write up to nBuf bytes of randomness into zBuf.
*/
static int xFreeRTOSRandomness(sqlite3_vfs *pVfs, int nBuf, char *zBuf){
  UNUSED_PARAMETER(pVfs);
  os_get_random((unsigned char*)zBuf, nBuf);
  return SQLITE_OK;
}


/*
** Sleep for a little while.  Return the amount of time slept.
*/
static int xFreeRTOSSleep(sqlite3_vfs *pVfs, int microsec){
  int milliseconds = (microsec + 999) / 1000;
  UNUSED_PARAMETER(pVfs);

  vTaskDelay((TickType_t)milliseconds);
  return milliseconds * 1000;
}

/*
** Find the current time (in Universal Coordinated Time).  Write into *piNow
** the current time and date as a Julian Day number times 86_400_000.  In
** other words, write into *piNow the number of milliseconds since the Julian
** epoch of noon in Greenwich on November 24, 4714 B.C according to the
** proleptic Gregorian calendar.
**
** On success, return SQLITE_OK.  Return SQLITE_ERROR if the time and date
** cannot be found.
*/
static int xFreeRTOSCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *prNow){
	static const sqlite3_int64 epoch = 24405875*(sqlite3_int64)8640000;
	int rc = SQLITE_OK;

	struct timeval sNow;
	(void)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
	*prNow = epoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
	return rc;
}



/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
static int xFreeRTOSCurrentTime(sqlite3_vfs *pVfs, double *prNow){
	  sqlite3_int64 i = 0;
	  int rc;
  UNUSED_PARAMETER(pVfs);
	  rc = xFreeRTOSCurrentTimeInt64(0, &i);
	  *prNow = i/86400000.0;
	  return rc;
}

/*
** The idea is that this function works like a combination of
** GetLastError() and FormatMessage() on windows (or errno and
** strerror_r() on unix). After an error is returned by an OS
** function, SQLite calls this function with zBuf pointing to
** a buffer of nBuf bytes. The OS layer should populate the
** buffer with a nul-terminated UTF-8 encoded error message
** describing the last IO error to have occurred within the calling
** thread.
**
** If the error message is too large for the supplied buffer,
** it should be truncated. The return value of xGetLastError
** is zero if the error message fits in the buffer, or non-zero
** otherwise (if the message was truncated). If non-zero is returned,
** then it is not necessary to include the nul-terminator character
** in the output buffer.
**
** Not supplying an error message will have no adverse effect
** on SQLite. It is fine to have an implementation that never
** returns an error message:
**
**   int xGetLastError(sqlite3_vfs *pVfs, int nBuf, char *zBuf){
**     assert(zBuf[0]=='\0');
**     return 0;
**   }
**
** However if an error message is supplied, it will be incorporated
** by sqlite into the error message available to the user using
** sqlite3_errmsg(), possibly making IO errors easier to debug.
*/
static int xFreeRTOSGetLastError(sqlite3_vfs *pVfs, int nBuf, char *zBuf){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(nBuf);
  UNUSED_PARAMETER(zBuf);
  return 0;
}

/*******************************************************************************
* Global Functions                                                             *
*******************************************************************************/
/*
** Initialize and deinitialize the operating system interface.
*/



static sqlite3_vfs FreeRTOSVfs = {
    3,                  		/* iVersion */
    sizeof(FatFsFile),			/* szOsFile */
	MAXPATHNAME,				/* mxPathname */
    0,                  		/* pNext */
    "FreeRTOSVFS",      		/* zName */
    0,                  		/* pAppData */
	xFreeRTOSOpen,          	/* xOpen */
	xFreeRTOSDelete,        	/* xDelete */
	xFreeRTOSAccess,        	/* xAccess */
	xFreeRTOSFullPathname,  	/* xFullPathname */
	pxFreeRTOSDlOpen,       	/* xDlOpen */
	xFreeRTOSDlError,       	/* xDlError */
	pxFreeRTOSDlSym,        	/* xDlSym */
	xFreeRTOSDlClose,       	/* xDlClose */
	xFreeRTOSRandomness,    	/* xRandomness */
	xFreeRTOSSleep,         	/* xSleep */
	xFreeRTOSCurrentTime,   	/* xCurrentTime */
	xFreeRTOSGetLastError,  	/* xGetLastError */
	xFreeRTOSCurrentTimeInt64,	/* xCurrentTimeInt64 */
    NULL,            			/* xSetSystemCall */
    NULL,            			/* xGetSystemCall */
    NULL,            			/* xNextSystemCall */
};



int sqlite_config_options(){
	int status = SQLITE_OK;
	/*	void* sqliteheap;
		sqliteheap = pvPortMalloc(40*1024);*/
/*		status = sqlite3_config(SQLITE_CONFIG_HEAP, xpBufSqlite_Heap, (80*1024), 32);
		if(status != SQLITE_OK)
			return SQLITE_ERROR;

		status = sqlite3_config(SQLITE_CONFIG_LOOKASIDE,0,0);
		if(status != SQLITE_OK)
			return SQLITE_ERROR;*/

		status = sqlite3_initialize();
		if(status != SQLITE_OK)
			return SQLITE_ERROR;
		//xpBufSqlite_Heap = aligned_alloc(8, (80*1024));

		return SQLITE_OK;
}
int sqlite3_os_init(void){

	int status = SQLITE_OK;
    /* Register fatfs file system. */
    sqlite3_vfs_register(&FreeRTOSVfs, 1);

    return status;
}

int sqlite3_os_end(void){
  return SQLITE_OK;
}
