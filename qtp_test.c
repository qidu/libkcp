#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include "qtpconnection.h"

#define FILENAME "/tmp/filename"
#define BUFSIZE 10000
#define RBUFSIZE 50000
#define PORT 9999

int main(int argc, char* argv[]) {
    QTPConnection *conn = NULL;
    if (argc == 1 || (argc > 1 && strcmp(argv[1], "127.0.0.1") == 0)) {
        conn = qtp_dial("127.0.0.1", PORT, true);
    }
    if (argc > 2) {
        conn = qtp_dial(argv[1], PORT, true);
    }
    if (conn == NULL) {
        printf("init session failed.[cmd ip op]\n");
        return -1;
    }
    
    // qtp_nodelay(conn, 1, 20, 2, 1);
    qtp_nodelay(conn, 0, 40, 0, 0);
    qtp_throughput(conn, 128*8, 128*8, 1400);
    qtp_setdscp(conn, 46);

    ssize_t nsent = 0;
    ssize_t nrecv = 0;
    ssize_t n = 0;
    int readed = 0, writed = 0, total = 0;

    if ((argc == 3 &&  strcmp(argv[2], "send") == 0)
    || (argc == 2 && strcmp(argv[1], "127.0.0.1") != 0 && strcmp(argv[2], "send") == 0)) {
        char buf[RBUFSIZE];
        FILE* file = fopen(FILENAME, "rb");
        if (!file ) {
            printf("open file %s failed\n", FILENAME);
        }
        printf("sending file data...\n");
        while ((readed = fread(buf, 1, BUFSIZE, file)) > 0) {
            writed = qtp_write(conn, buf, readed);
            total += writed;
            printf("read %d send %d total %d\n", readed, writed, total);
            qtp_update(conn);
            do {                
                n = qtp_read(conn, buf, BUFSIZE);
            } while(n > 0);
            if (n <= 0) {
                usleep(33000);         
            }
        }
        qtp_write(conn, "bye", 3);
        qtp_update(conn);
        return 0;
    }

    printf("recving file data...\n");
    char rbuf[RBUFSIZE];
    n = qtp_write(conn, FILENAME, sizeof(FILENAME)); // write filename trigger response from server side
    while (1) {
        qtp_update(conn);
        n = qtp_read(conn, rbuf, RBUFSIZE);
        if (n > 0) {
            total += n;
            printf("recv %lu total %d\n", n, total);
            if (strncmp(&rbuf[n-3], "bye", 3) == 0) {
                qtp_write(conn, "bye", 3);
                qtp_update(conn);
                sleep(1);
                break;
            }
        }
        else {
            usleep(33000);
        }
    }

    printf("recv total %d\n", total);
    qtp_close(conn);
}
