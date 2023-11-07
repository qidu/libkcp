/* ./proxy_server -t www.qiniuapi.com:80 --key 1234
 *  sudo -E ./proxy_client -l :7890 -r 127.0.0.1:29900 --key 1234
 *  curl -H "Host: qiniuapi.com" http://localhost:7890/
 */
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "qtpconnection.h"

#define RBUFSIZE 50000
#define PROXY_PORT "29900"
#define DEST_REQ "GET /\r\n/Host: qiniuapi.com\r\nAccept: */*\r\n\r\n"

int main(int argc, char* argv[]) {
    QTPConnection *conn = NULL;
    if (argc > 1) {
		char *host = argv[1];
		char *port = strchr(host, ':');
		if (port != NULL) {
			*port = '\0';
			port++;
		}
		else {
			port = PROXY_PORT;
		}
		conn = qtp_dial(host, atoi(port), 0x1234, true);
    }
    else 
    {
        conn = qtp_dial("127.0.0.1", 29900, 0x1234, true);
    }
    
    // qtp_nodelay(conn, 1, 20, 2, 1);
    qtp_nodelay(conn, 0, 40, 0, 0);
    qtp_throughput(conn, 128*8, 128*8, 1400);
    qtp_setdscp(conn, 46);

    ssize_t nsent = 0;
    ssize_t nrecv = 0;
    ssize_t n = 0;
    int readed = 0, writed = 0, total = 0;

    printf("recving file data...\n");
    char rbuf[RBUFSIZE];
    n = qtp_write(conn, DEST_REQ, sizeof(DEST_REQ)); // write GET
    while (1) {
        qtp_update(conn);
        n = qtp_read(conn, rbuf, RBUFSIZE);
        if (n > 0) {
            total += n;
            // printf("recv %lu total %d\n", n, total);
			rbuf[n] = '\0';
			printf("%s", rbuf);
        }
        else {
            usleep(33000);
        }
    }

    printf("recv total %d\n", total);
    qtp_close(conn);
}
