/*
 * Copyright 2014 Red Hat, Inc.  All rights reserved.
 * Copyright 2015 Milosz Tanski
 *
 * License: GPLv2
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <linux/fs.h> /* for RWF_NONBLOCK */

/*
 * Once preadv2 is part of the upstream kernel and there is glibc support for
 * it. We'll add support for preadv2 to xfs_io and this will be unnecessary.
 */
#include "preadv2-pwritev2.h"

/*
 * Test to see if the system call is implemented.  If -EINVAL or -ENOSYS
 * are returned, consider the call unimplemented.  All other errors are
 * considered success.
 *
 * Returns: 0 if the system call is implemented, 1 if the system call
 * is not implemented.
 */
int
preadv2_check(int fd)
{
	int ret;
	struct iovec iov[] = {};

	/* 0 length read; just check iof the syscall is there.
         *
         * - 0 length iovec
         * - Position is -1 (eg. use current position)
         */
	ret = preadv2(fd, iov, 0, -1, 0);

	if (ret < 0) {
		if (errno == ENOSYS || errno == EINVAL)
			return 1;
	}

	return 0;
}

void
usage(char *prog)
{
	fprintf(stderr, "Usage: %s [-v] [-ctdw] [-n] -p POS -l LEN <filename>\n\n", prog);
	fprintf(stderr, "General arguments:\n");
	fprintf(stderr, "  -v Verify that the syscall is supported and quit:\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Open arguments:\n");
	fprintf(stderr, "  -c Open file with O_CREAT flag\n");
	fprintf(stderr, "  -t Open file with O_TRUNC flag\n");
	fprintf(stderr, "  -d Open file with O_DIRECT flag\n");
	fprintf(stderr, "  -w Open file with O_RDWR flag vs O_RDONLY (default)\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "preadv2 arguments:\n");
	fprintf(stderr, "  -n use RWF_NONBLOCK when performing read\n");
	fprintf(stderr, "  -p POS offset file to read at\n");
	fprintf(stderr, "  -l LEN length of file data to read\n");
	fprintf(stderr, "\n");
	fflush(stderr);
}

int
main(int argc, char **argv)
{
	int fd;
	int ret;
	int opt;
	off_t pos = -1;
	struct iovec iov = { NULL, 0 };
	int o_flags = 0;
	int r_flags = 0;
	char *filename;

	while ((opt = getopt(argc, argv, "vctdwnp:l:")) != -1) {
		switch (opt) {
		case 'v':
			/*
			 * See if we were called to check for availability of
			 * sys_preadv2. STDIN is okay, since we do a zero
			 * length read (see man 2 read).
			 */
			ret = preadv2_check(STDIN_FILENO);
			exit(ret);
		case 'c':
			o_flags |= O_CREAT;
			break;
		case 't':
			o_flags |= O_TRUNC;
			break;
		case 'd':
			o_flags |= O_DIRECT;
			break;
		case 'w':
			o_flags |= O_RDWR;
			break;
		case 'n':
			r_flags |= RWF_NONBLOCK;
			break;
		case 'p':
			pos = atoll(optarg);
			break;
		case 'l':
			iov.iov_len = atoll(optarg);
			break;
		default:
			fprintf(stderr, "invalid option: %c\n", opt);
			usage(argv[0]);
			exit(1);
		}
	}

	if (optind >= argc) {
		usage(argv[0]);
		exit(1);
	}

	if ((o_flags & O_RDWR) != O_RDWR)
		o_flags |= O_RDONLY;

	if ((iov.iov_base = malloc(iov.iov_len)) == NULL) {
		perror("malloc");
		exit(1);
	}

	filename = argv[optind];
	fd = open(filename, o_flags);

	if (fd < 0) {
		perror("open");
		exit(1);
	}

	if ((ret = preadv2(fd, &iov, 1, pos, r_flags)) == -1) {
		perror("preadv2");
		exit(ret);
	}

	free(iov.iov_base);
	exit(0);
}
