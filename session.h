#ifndef KCP_SESS_H
#define KCP_SESS_H

#include "ikcp.h"
#include <sys/types.h>
#include <sys/time.h>
#include <stdint.h>

typedef unsigned char byte;

typedef struct {
    int sockfd{0};
    ikcpcb *kcp{nullptr};
    byte buf[2048];
    byte streambuf[65535];
    size_t streambufsiz{0};
    uint32_t pktidx{0};
} QTPConnection;

QTPConnection *qtp_dial(const char *ip, uint16_t port, bool stream);
void qtp_update(QTPConnection* conn);
ssize_t qtp_read(QTPConnection* conn, char *buf, size_t sz);
ssize_t qtp_write(QTPConnection* conn, const char *buf, size_t sz);
int qtp_setdscp(QTPConnection* conn, int dscp);
int qtp_nodelay(QTPConnection* conn, int nodelay, int interval, int resend, int nc);
int qtp_throughput(QTPConnection* conn, int sndwnd, int rcvwnd, int mtu);
void qtp_close(QTPConnection *conn);
uint32_t qtp_current(); // ms

#endif //KCP_SESS_H
