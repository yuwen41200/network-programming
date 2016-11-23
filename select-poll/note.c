#include <sys/types.h>
#include <sys/socket.h>

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

/**
 * commonly used socket options:
 * SO_KEEPALIVE, SO_RCVBUF, SO_SNDBUF, SO_REUSEADDR, SO_LINGER
 */

#include <unistd.h>
#include <fcntl.h>

int fcntl(int fd, int cmd, ... /* arg */ );

/**
 * commonly used file descriptor operations:
 * F_GETFL, F_SETFL, F_GETOWN, F_SETOWN
 */
