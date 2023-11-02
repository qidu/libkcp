#include <unistd.h>
#include <sys/time.h>
#include <cstring>
#include <cstdio>
#include "session.h"

#define FILENAME "/tmp/filename"
#define BUFSIZE 10000
#define RBUFSIZE 50000
#define PORT 9999

IUINT32 iclock();

int main(int argc, char* argv[]) {
    struct timeval time;
    gettimeofday(&time, NULL);
    srand((time.tv_sec * 1000) + (time.tv_usec / 1000));

    KCPSession *sess = NULL;
    if (argc == 1 || (argc > 1 && strcmp(argv[1], "127.0.0.1") == 0)) {
        sess = KCPSession::Dial("127.0.0.1", PORT);
    }
    if (argc > 2) {
        sess = KCPSession::Dial(argv[1], PORT);
    }
    if (sess == NULL) {
        printf("init session failed.\n");
        return -1;
    }
    // sess->NoDelay(1, 20, 2, 1);
    sess->NoDelay(0, 40, 0, 0);
    sess->WndSize(128*8, 128*8);
    sess->SetMtu(1400);
    sess->SetStreamMode(true);
    sess->SetDSCP(46);

    assert(sess != nullptr);
    ssize_t nsent = 0;
    ssize_t nrecv = 0;
    ssize_t n = 0;
    int readed = 0, writed = 0, total = 0;

    // char *buf = (char *) malloc(128);
    if ((argc == 3 &&  strcmp(argv[2], "send") == 0)
    || (argc == 2 && strcmp(argv[1], "127.0.0.1") != 0 && strcmp(argv[2], "send") == 0)) {
        char buf[RBUFSIZE];
        FILE* file = fopen(FILENAME, "rb");
        if (!file ) {
            printf("open file %s failed\n", FILENAME);
        }
        printf("sending file data...\n");
        while ((readed = fread(buf, 1, BUFSIZE, file)) > 0) {
            writed = sess->Write(buf, readed);
            total += writed;
            printf("read %d send %d total %d\n", readed, writed, total);
            sess->Update(iclock());
            do {                
                n = sess->Read(buf, BUFSIZE);
            } while(n > 0);
            if (n <= 0) {
                usleep(33000);         
            }
        }
        sess->Write("bye", 3);
        sess->Update(iclock());
        return 0;
    }

    printf("recving file data...\n");
    char rbuf[RBUFSIZE];
    n = sess->Write(FILENAME, sizeof(FILENAME)); // write filename trigger response from server side
    while (1) {
        sess->Update(iclock());
        n = sess->Read(rbuf, RBUFSIZE);
        if (n > 0) {
            total += n;
            printf("recv %lu total %d\n", n, total);
            if (strncmp(&rbuf[n-3], "bye", 3) == 0) {
                break;
            }
        }
        else {
            usleep(33000);
        }
    }

    printf("recv total %d\n", total);
    KCPSession::Destroy(sess);
}


void
itimeofday(long *sec, long *usec) {
    struct timeval time;
    gettimeofday(&time, NULL);
    if (sec) *sec = time.tv_sec;
    if (usec) *usec = time.tv_usec;
}

IUINT64 iclock64(void) {
    long s, u;
    IUINT64 value;
    itimeofday(&s, &u);
    value = ((IUINT64) s) * 1000 + (u / 1000);
    return value;
}

IUINT32 iclock() {
    return (IUINT32) (iclock64() & 0xfffffffful);
}


