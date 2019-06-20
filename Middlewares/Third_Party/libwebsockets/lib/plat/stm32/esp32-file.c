/*
 * libwebsockets - lib/plat/lws-plat-esp32.c
 *
 * Copyright (C) 2010-2017 Andy Green <andy@warmcat.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation:
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */

#include "core/private.h"
#include "fatfs_wrapper.h"

int lws_plat_apply_FD_CLOEXEC(int n)
{
	return 0;
}


LWS_VISIBLE lws_fop_fd_t
_lws_plat_file_open(const struct lws_plat_file_ops *fops, const char *filename,
		    const char *vpath, lws_fop_flags_t *flags)
{
	struct stat stat_buf;
	lws_fop_fd_t fop_fd;
	int ret = fat_open(filename, *flags, 0664);

	if (ret < 0)
		return NULL;

	if (fstat(ret, &stat_buf) < 0)
		goto bail;

	fop_fd = lws_malloc(sizeof(*fop_fd), "fops open");
	if (!fop_fd)
		goto bail;

	fop_fd->fops = fops;
	fop_fd->fd = ret;
	fop_fd->flags = *flags;
	fop_fd->filesystem_priv = NULL; /* we don't use it */
	fop_fd->pos = 0;
	fop_fd->len = stat_buf.st_size;

	return fop_fd;

bail:
	close(ret);

	return NULL;
}

LWS_VISIBLE int
_lws_plat_file_close(lws_fop_fd_t *fops_fd)
{
	int fd = (*fops_fd)->fd;

	lws_free(*fops_fd);
	*fops_fd = NULL;

	return fat_close(fd);
}

LWS_VISIBLE lws_fileofs_t
_lws_plat_file_seek_cur(lws_fop_fd_t fop_fd, lws_fileofs_t offset)
{
	lws_fileofs_t r;

	if (offset > 0 &&
	    offset > (lws_fileofs_t)fop_fd->len - (lws_fileofs_t)fop_fd->pos)
		offset = fop_fd->len - fop_fd->pos;

	if ((lws_fileofs_t)fop_fd->pos + offset < 0)
		offset = -fop_fd->pos;

	r = lseek(fop_fd->fd, offset, SEEK_CUR);

	if (r >= 0)
		fop_fd->pos = r;
	else
		lwsl_err("error seeking from cur %ld, offset %ld\n",
                        (long)fop_fd->pos, (long)offset);

	return r;
}

LWS_VISIBLE int
_lws_plat_file_read(lws_fop_fd_t fops_fd, lws_filepos_t *amount,
		    uint8_t *buf, lws_filepos_t len)
{
	long n;

	n = fat_read(fops_fd->fd, buf, len);
	if (n == -1) {
		*amount = 0;
		return -1;
	}
	fops_fd->pos += n;
	*amount = n;

	return 0;
}

LWS_VISIBLE int
_lws_plat_file_write(lws_fop_fd_t fops_fd, lws_filepos_t *amount,
		     uint8_t *buf, lws_filepos_t len)
{
	long n;

	n = fat_write(fops_fd->fd, buf, len);
	if (n == -1) {
		*amount = 0;
		return -1;
	}
	fops_fd->pos += n;
	*amount = n;

	return 0;
}

#if defined(LWS_AMAZON_RTOS) || defined(LWS_WITH_STM32)
int
lws_find_string_in_file(const char *filename, const char *string, int stringlen)
{
    return 0;
}
#else
int
lws_find_string_in_file(const char *filename, const char *string, int stringlen)
{
	char buf[128];
	int fd, match = 0, pos = 0, n = 0, hit = 0;

	fd = fat_open(filename, O_RDONLY);
	if (fd < 0) {
		lwsl_err("can't open auth file: %s\n", filename);
		return 0;
	}

	while (1) {
		if (pos == n) {
			n = fat_read(fd, buf, sizeof(buf));
			if (n <= 0) {
				if (match == stringlen)
					hit = 1;
				break;
			}
			pos = 0;
		}

		if (match == stringlen) {
			if (buf[pos] == '\r' || buf[pos] == '\n') {
				hit = 1;
				break;
			}
			match = 0;
		}

		if (buf[pos] == string[match])
			match++;
		else
			match = 0;

		pos++;
	}

	fat_close(fd);

	return hit;
}
#endif

#if !defined(LWS_AMAZON_RTOS) && !defined(LWS_WITH_STM32)
LWS_VISIBLE int
lws_plat_write_file(const char *filename, void *buf, int len)
{
	int m, fd;

	fd = fat_open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd == -1)
		return 1;

	m = fat_write(fd, buf, len);
	fat_close(fd);

	return m != len;
}

/* we write vhostname.cert.pem and vhostname.key.pem, 0 return means OK */

LWS_VISIBLE int
lws_plat_write_cert(struct lws_vhost *vhost, int is_key, int fd, void *buf,
			int len)
{
	const char *name = vhost->tls.alloc_cert_path;

	if (is_key)
		name = vhost->tls.key_path;

	return lws_plat_write_file(name, buf, len) < 0;
}

LWS_VISIBLE int
lws_plat_read_file(const char *filename, void *buf, int len)
{
	int n, fd = fat_open(filename, O_RDONLY);
	if (fd == -1)
		return -1;

	n = fat_read(fd, buf, len);
	fat_close(fd);

	return n;
}
#endif /* LWS_AMAZON_RTOS */
