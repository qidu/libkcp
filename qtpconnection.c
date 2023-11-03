#include "qtpconnection.h"
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

static QTPConnection* dialIPv6(const char *ip, uint16_t port, bool stream);
static QTPConnection *new_connection(int sockfd, bool stream);
static ssize_t output(QTPConnection *conn, const void *buffer, size_t length);
static int out_wrapper(const char *buf, int len, struct IKCPCB *kcp, void *user);

uint32_t qtp_current() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return (uint32_t)(time.tv_sec * 1000) + (time.tv_usec / 1000);
}

static void itimeofday(long *sec, long *usec) {
    struct timeval time;
    gettimeofday(&time, NULL);
    if (sec) *sec = time.tv_sec;
    if (usec) *usec = time.tv_usec;
}

static IUINT64 iclock64(void) {
    long s, u;
    IUINT64 value;
    itimeofday(&s, &u);
    value = ((IUINT64) s) * 1000 + (u / 1000);
    return value;
}

static IUINT32 iclock() {
    return (IUINT32) (iclock64() & 0xfffffffful);
}

void qtp_setstream(QTPConnection* conn, bool yes) {
    if (yes) {
        conn->kcp->stream = 1;
    } else {
        conn->kcp->stream = 0;
    }
}

int qtp_setdscp(QTPConnection *conn, int iptos) {
    iptos = (iptos << 2) & 0xFF;
    return setsockopt(conn->sockfd, IPPROTO_IP, IP_TOS, &iptos, sizeof(iptos));
}

ssize_t qtp_write(QTPConnection *conn, const char *buf, size_t sz) {
    int n = ikcp_send(conn->kcp, buf, (int)sz);
    if (n == 0) {
        return sz;
    } else return n;
}

ssize_t qtp_read(QTPConnection *conn, char *buf, size_t sz) {
    if (conn->streambufsiz > 0) {
        size_t n = conn->streambufsiz;
        if (n > sz) {
            n = sz;
        }
        memcpy(buf, conn->streambuf, n);

        conn->streambufsiz -= n;
        if (conn->streambufsiz != 0) {
            memmove(conn->streambuf, conn->streambuf + n, conn->streambufsiz);
        }
        return n;
    }

    int psz = ikcp_peeksize(conn->kcp);
    if (psz <= 0) {
        return 0;
    }

    if (psz <= sz) {
        return (ssize_t) ikcp_recv(conn->kcp, buf, (int)sz);
    } else {
        ikcp_recv(conn->kcp, (char *) conn->streambuf, sizeof(conn->streambuf));
        memcpy(buf, conn->streambuf, sz);
        conn->streambufsiz = psz - sz;
        memmove(conn->streambuf, conn->streambuf + sz, psz - sz);
        return sz;
    }
}

void qtp_update(QTPConnection *conn) {
    for (;;) {
        ssize_t n = recv(conn->sockfd, conn->buf, sizeof(conn->buf), 0);
        if (n > 0) {
            // fec disabled
            ikcp_input(conn->kcp, (char *) (conn->buf), n);
        } else {
            break;
        }
    }
    conn->kcp->current = iclock();
    ikcp_flush(conn->kcp);
}

ssize_t output(QTPConnection *conn, const void *buffer, size_t length) {
    ssize_t n = send(conn->sockfd, buffer, length, 0);
    return n;
}

int out_wrapper(const char *buf, int len, struct IKCPCB *kcp, void *user) {
    if (!kcp || !user || !buf || len <= 0) {
        return -1;
    }
    QTPConnection *conn = (QTPConnection *)user;
    // No FEC, just send raw bytes,
    output(conn, buf, len);
    return 0;
}

void qtp_close(QTPConnection *conn) {
    if (NULL == conn) return;
    if (0 != conn->sockfd) { close(conn->sockfd); }
    if (NULL != conn->kcp) { ikcp_release(conn->kcp); }
}

int qtp_throughput(QTPConnection* conn, int sndwnd, int rcvwnd, int mtu) {
    ikcp_wndsize(conn->kcp, sndwnd, rcvwnd);
    return ikcp_setmtu(conn->kcp, mtu);
}

int qtp_nodelay(QTPConnection* conn, int nodelay, int interval, int resend, int nc) {
    return ikcp_nodelay(conn->kcp, nodelay, interval, resend, nc);
}

QTPConnection *new_connection(int sockfd, bool stream) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0) {
        return NULL;
    }

    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        return NULL;
    }

    QTPConnection *conn = (QTPConnection*)malloc(sizeof(QTPConnection));
    if (!conn) {
        return NULL;
    }
    conn->sockfd = sockfd;
    conn->kcp = ikcp_create((IUINT32)rand(), conn);
    conn->kcp->output = out_wrapper;
    conn->kcp->stream = stream ? 1 : 0;
    return conn;
}

QTPConnection *qtp_dial(const char *ip, uint16_t port, bool stream) {
    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    int ret = inet_pton(AF_INET, ip, &(saddr.sin_addr));

    if (ret == 1) { // do nothing
    } else if (ret == 0) { // try ipv6
        return dialIPv6(ip, port, stream);
    } else if (ret == -1) {
        return NULL;
    }

    int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        return NULL;
    }
    if (connect(sockfd, (struct sockaddr *) &saddr, sizeof(struct sockaddr)) < 0) {
        close(sockfd);
        return NULL;
    }

    return new_connection(sockfd, stream);
}

QTPConnection *dialIPv6(const char *ip, uint16_t port, bool stream) {
    struct sockaddr_in6 saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin6_family = AF_INET6;
    saddr.sin6_port = htons(port);
    if (inet_pton(AF_INET6, ip, &(saddr.sin6_addr)) != 1) {
        return NULL;
    }

    int sockfd = socket(PF_INET6, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        return NULL;
    }
    if (connect(sockfd, (struct sockaddr *) &saddr, sizeof(struct sockaddr_in6)) < 0) {
        close(sockfd);
        return NULL;
    }

    return new_connection(sockfd, stream);
}
