#ifndef PREADV2_PWRITEV2_H
#define PREADV2_PWRITEV2_H

#include "global.h"

#ifndef HAVE_PREADV2
#include <sys/syscall.h>

#if !defined(SYS_preadv2) && defined(__x86_64__)
#define SYS_preadv2 323
#define SYS_pwritev2 324
#endif

#if !defined (SYS_preadv2) && defined(__i386__)
#define SYS_preadv2 359
#define SYS_pwritev2 360
#endif

/* LO_HI_LONG taken from glibc */
#define LO_HI_LONG(val)							\
  (off_t) val,                                                          \
  (off_t) ((((uint64_t) (val)) >> (sizeof (long) * 4)) >> (sizeof (long) * 4))

static inline ssize_t
preadv2(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags)
{
#ifdef SYS_preadv2
        return syscall(SYS_preadv2, fd, iov, iovcnt, LO_HI_LONG(offset),
		       flags);
#else
	errno = ENOSYS;
	return -1;
#endif
}

static inline ssize_t
pwritev2(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags)
{
#ifdef SYS_pwritev2
        return syscall(SYS_pwritev2, fd, iov, iovcnt, LO_HI_LONG(offset),
		       flags);
#else
	errno = ENOSYS;
	return -1;
#endif
}

#define RWF_NONBLOCK	0x00000001
#define RWF_DSYNC	0x00000002

#endif /* HAVE_PREADV2 */
#endif /* PREADV2_PWRITEV2_H */
