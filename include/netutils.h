#ifndef _NETUTILS_H
#define _NETUTILS_H

#include <errno.h>
#include <unistd.h>

ssize_t	forceread(int fd, void *ptr, size_t size, bool text_mode = true) {
	size_t size_left = size;
	ssize_t	size_read;
	while (size_left > 0) {
		size_read = read(fd, ptr, size_left);
		if (size_read < 0) {
			if (errno == EINTR)
				size_read = 0;
			else
				return -1;
		}
		else if (size_read == 0)
			break;
		size_left -= size_read;
		ptr = (char *) ptr + size_read;
		if (text_mode && *((char *) ptr - 1) == 0)
			break;
	}
	return size - size_left;
}

ssize_t	forcewrite(int fd, const void *ptr, size_t size) {
	size_t size_left = size;
	ssize_t size_written;
	while (size_left > 0) {
		size_written = write(fd, ptr, size_left);
		if (size_written < 0) {
			if (errno == EINTR)
				size_written = 0;
			else
				return -1;
		}
		else if (size_written == 0)
			return -1;
		size_left -= size_written;
		ptr = (char *) ptr + size_written;
	}
	return size - size_left;
}

#endif
