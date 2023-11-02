#ifndef KCP_SESS_H
#define KCP_SESS_H

#include "ikcp.h"
#include <sys/types.h>
#include <sys/time.h>
#include <cstdint>

typedef unsigned char byte;

class KCPSession  {
private:
    int m_sockfd{0};
    ikcpcb *m_kcp{nullptr};
    byte m_buf[2048];
    byte m_streambuf[65535];
    size_t m_streambufsiz{0};

    uint32_t pkt_idx{0};
public:
    KCPSession(const KCPSession &) = delete;

    KCPSession &operator=(const KCPSession &) = delete;

    // Dial connects to the remote server and returns KCPSession.
    static KCPSession *Dial(const char *ip, uint16_t port);

    // Update will try reading/writing udp packet, pass current unix millisecond
    void Update(uint32_t current) noexcept;

    // Destroy release all resource related.
    static void Destroy(KCPSession *sess);

    // Read reads from kcp with buffer empty sz.
    ssize_t Read(char *buf, size_t sz) noexcept;

    // Write writes into kcp with buffer empty sz.
    ssize_t Write(const char *buf, size_t sz) noexcept;

    // Set DSCP value
    int SetDSCP(int dscp) noexcept;

    // SetStreamMode toggles the stream mode on/off
    void SetStreamMode(bool enable) noexcept;

    // Wrappers for kcp control
    inline int NoDelay(int nodelay, int interval, int resend, int nc) {
        return ikcp_nodelay(m_kcp, nodelay, interval, resend, nc);
    }

    inline int WndSize(int sndwnd, int rcvwnd) { return ikcp_wndsize(m_kcp, sndwnd, rcvwnd); }

    inline int SetMtu(int mtu) { return ikcp_setmtu(m_kcp, mtu); }

private:
    KCPSession() = default;

    ~KCPSession() = default;

    // DialIPv6 is the ipv6 version of Dial.
    static KCPSession *dialIPv6(const char *ip, uint16_t port);

    // out_wrapper
    static int out_wrapper(const char *buf, int len, struct IKCPCB *kcp, void *user);

    // output udp packet
    ssize_t output(const void *buffer, size_t length);

    static KCPSession *createSession(int sockfd);


};

inline uint32_t currentMs() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return uint32_t((time.tv_sec * 1000) + (time.tv_usec / 1000));
}


#endif //KCP_SESS_H
