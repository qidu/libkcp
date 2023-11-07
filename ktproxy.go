package main

import (
	"fmt"
	"net"
	"os"
	"time"

	"github.com/qidu/ktp-go/v6"
)

const port = ":29900"

func ListenTest() (*kcp.Listener, error) {
	usefec := os.Getenv("FEC")
	if usefec == "" {
	        fmt.Println("init without fec.")
		return kcp.ListenWithOptions(port, nil, 0, 0, 0x1234)
	}
	fmt.Println("init with fec(2,2)")
	return kcp.ListenWithOptions(port, nil, 2, 2, 0)
}

func server() {
	l, err := ListenTest()
	if err != nil {
		panic(err)
	}
	ch := make(chan int)
	done := 0
	l.SetDSCP(46)
	for {
		l.SetDeadline(time.Now().Add(1 * time.Second))
		conn, err := l.AcceptKCP()
		select {
            case msg := <-ch:
                if msg == 1 {
					done += 1
					if (done >= 2) {
                    	fmt.Println("finish all.")
                    	return
					}
                }
            default:
        }
		if err != nil {
			fmt.Println(err)
			continue
		}
		conn.SetStreamMode(true)
		conn.SetNoDelay(0, 40, 0, 0)
		// conn.SetNoDelay(1, 20, 2, 1)
		conn.SetWindowSize(1024, 1024)
		go handle_client(conn, string("www.qiniuapi.com:80"), ch)
	}
}

func handle_client(conn *kcp.UDPSession, target string, ch chan int) {
	count := 0
	out_total := 0
	in_total := 0
	// writed := 0
	
    upstream, _ := net.Dial("tcp", target)
	fmt.Println("new client", conn.RemoteAddr(), "to", upstream.RemoteAddr())

	streamOut := func(stream net.Conn, conn *kcp.UDPSession, ch chan int) {
		buf := make([]byte, 65536)
		for {
			count++
			conn.SetDeadline(time.Now().Add(1 * time.Second))
			n, err := conn.Read(buf)
			if err != nil {
				fmt.Println("finish read")
				ch <- 1
				break
			}
			stream.Write(buf[0:n])	
			out_total += n
			fmt.Printf("count %d receive %d total %d\n", count, n, out_total)
		}
	}

	streamIn := func(stream net.Conn, conn *kcp.UDPSession, ch chan int) {
		buf := make([]byte, 65536)
        for {
            count++
            n, err := stream.Read(buf)
            if err != nil {
				ch <- 1
				fmt.Println("finish upstream.")
				break
            }
            conn.Write(buf[0:n])
            in_total += n
            fmt.Printf("count %d receive %d total %d\n", count, n, in_total)
        }
	}

	go streamIn(upstream, conn, ch)
	streamOut(upstream, conn, ch)
}

func main() {
	server()
}
