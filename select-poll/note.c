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

#include <sys/socket.h>
#include <netdb.h>

struct hostent *gethostbyname(const char *name);
struct hostent *gethostbyname2(const char *name, int family);
struct hostent *gethostbyaddr(const void *addr, socklen_t len, int family);

int gethostbyname_r(const char *name,
                    struct hostent *ret, char *buf, size_t buflen,
                    struct hostent **result, int *h_errnop);
int gethostbyname2_r(const char *name, int family,
                     struct hostent *ret, char *buf, size_t buflen,
                     struct hostent **result, int *h_errnop);
int gethostbyaddr_r(const void *addr, socklen_t len, int family,
                    struct hostent *ret, char *buf, size_t buflen,
                    struct hostent **result, int *h_errnop);

struct servent *getservbyname(const char *name, const char *proto);
struct servent *getservbyport(int port, const char *proto);

/**
 * foo_r():
 * reentrant version of foo()
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(const char *host, const char *serv,
                const struct addrinfo *hints, struct addrinfo **res);
void freeaddrinfo(struct addrinfo *res);
int getnameinfo(const struct sockaddr *sa, socklen_t salen,
                char *host, socklen_t hostlen,
                char *serv, socklen_t servlen, int flags);

/**
 * newer conversion functions
 */

#include <unistd.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>

ssize_t read(int fd, void *buf, size_t len);
ssize_t write(int fd, const void *buf, size_t len);

ssize_t readv(int fd, const struct iovec *iov, int iov_len);
ssize_t writev(int fd, const struct iovec *iov, int iov_len);

ssize_t recv(int fd, void *buf, size_t len, int flags);
ssize_t send(int fd, const void *buf, size_t len, int flags);

ssize_t recvfrom(int fd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addr_len);
ssize_t sendto(int fd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addr_len);

ssize_t recvmsg(int fd, struct msghdr *msg, int flags);
ssize_t sendmsg(int fd, const struct msghdr *msg, int flags);

/**
 * comparison of socket I/O functions
 */
