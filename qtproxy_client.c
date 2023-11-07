/* go run ../ktproxy.go "key=0x1234"
 * ./qtproxy_client
 * curl -v http://www.qiniuapi.com/ -o /dev/null
 */
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "qtpconnection.h"

#define RBUFSIZE 50000
#define PROXY_PORT "29900"
#define DEST_REQ "GET / HTTP/1.1\r\nHost: www.qiniuapi.com\r\nUser-Agent: qtp/1.0\r\nAccept: */*\r\n\r\n"

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

    char rbuf[RBUFSIZE];
    n = qtp_write(conn, DEST_REQ, strlen(DEST_REQ)); // write GET
    while (1) {
        qtp_update(conn);
        n = qtp_read(conn, rbuf, RBUFSIZE);
        if (n > 0) {
			if (strncmp(rbuf, "bye", 3) == 0) {
				break;
			}
            total += n;
			rbuf[n] = '\0';
			printf("%s", rbuf);
            if (strstr(rbuf, "\r\n\r")) {
                break;
            }
        }
        else {
            usleep(33000);
        }
    }

    fprintf(stderr, "\n\n===== recv total %d ====\n", total);
    qtp_close(conn);
}
