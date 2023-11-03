#ifndef QTP_CONN_H
#define QTP_CONN_H

#include <sys/types.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdbool.h>

typedef unsigned char byte;

typedef struct {
    int sockfd;
    void *handle;
    byte buf[2048];
    byte streambuf[65535];
    size_t streambufsiz;
    uint32_t pktidx;
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

#endif //QTP_CONN_H
