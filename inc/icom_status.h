#ifndef _ICOM_STATUS_H_
#define _ICOM_STATUS_H_

#include "icom_type.h"

typedef enum {
  ICOM_SUCCESS=0,    /** SUCCESS */
  ICOM_EPERM,        /** Not owner */
  ICOM_ENOENT,       /** No such file or directory */
  ICOM_ESRCH,        /** No such context */
  ICOM_EINTR,        /** Interrupted system call */
  ICOM_EIO,          /** I/O error */
  ICOM_ENXIO,        /** No such device or address */
  ICOM_E2BIG,        /** Arg list too long */
  ICOM_ENOEXEC,      /** Exec format error */
  ICOM_EBADF,        /** Bad file number */
  ICOM_ECHILD,       /** No children */
  ICOM_EAGAIN,       /** No more contexts */
  ICOM_ENOMEM,       /** Not enough core */
  ICOM_EACCES,       /** Permission denied */
  ICOM_EFAULT,       /** Bad address */
  ICOM_ENOTEMPTY,    /** Directory not empty */
  ICOM_EBUSY,        /** Mount device busy */
  ICOM_EEXIST,       /** File exists */
  ICOM_EXDEV,        /** Cross-device link */
  ICOM_ENODEV,       /** No such device */
  ICOM_ENOTDIR,      /** Not a directory */
  ICOM_EISDIR,       /** Is a directory */
  ICOM_EINVAL,       /** Invalid argument */
  ICOM_ENFILE,       /** File table overflow */
  ICOM_EMFILE,       /** Too many open files */
  ICOM_ENOTTY,       /** Not a typewriter */
  ICOM_ENAMETOOLONG, /** File name too long */
  ICOM_EFBIG,        /** File too large */
  ICOM_ENOSPC,       /** No space left on device */
  ICOM_ESPIPE,       /** Illegal seek */
  ICOM_EROFS,        /** Read-only file system */
  ICOM_EMLINK,       /** Too many links */
  ICOM_EPIPE,        /** Broken pipe */
  ICOM_EDEADLK,      /** Resource deadlock avoided */
  ICOM_ENOLCK,       /** No locks available */
  ICOM_ENOTSUP,      /** Unsupported value */
  ICOM_EMSGSIZE,     /** Message size */

  /* Custom ICOM specific error codes */
  ICOM_ITYPE,        /** Invalid type */
  ICOM_IFLAGS,       /** Invalid flags */
  ICOM_NIMPL,        /** Not implemented */
  ICOM_ERROR,        /** General error */
  ICOM_ELINK,        /** Failed to initialize link */
  ICOM_ELOOKUP,      /** Look up error */
  ICOM_ECONNREFUSED, /** Refused connection */
  ICOM_PARTIAL,      /** Data sent partially */
  ICOM_TIMEOUT,      /** Timeout condition */

  /* Number of valid codes, required for PTR_ERR and IS_ERR */
  ICOM_COUNT
} icomStatus_t;


/** @def ICOM_IS_ERR(x)
 *  Checks if the x pointer is actually an error */
#define ICOM_IS_ERR(x)   ((long long unsigned)x < ICOM_COUNT)

/** @def ICOM_PTR_ERR(x)
 * Casts (in future may extract) x pointer to a status code (icomStatus_t) */
#define ICOM_PTR_ERR(x)  ((icomStatus_t)x)


#endif
