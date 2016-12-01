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
