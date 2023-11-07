package main

import (
	"fmt"
	"net"
	"bufio"
	"time"

	"github.com/qidu/ktp-go/v6"
)

const port = ":29900"

func ListenTest() (*kcp.Listener, error) {
    return kcp.ListenWithOptions(port, nil, 0, 0, 0x1234) // key 0x1234
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
	out_total := 0
	// in_total := 0
	// writed := 0
	
    upstream, err := net.Dial("tcp", target)
	fmt.Println("new client from", conn.RemoteAddr(), "to", upstream.RemoteAddr())
	if err != nil {
		fmt.Println("dial remote error.")
		ch <- 1
		ch <- 1
		return
	}
	defer upstream.Close()


	streamOut := func(stream net.Conn, conn *kcp.UDPSession, ch chan int) {
        //request := "GET / HTTP/1.1\r\nHost: www.qiniuapi.com\r\n\r\n"
		//fmt.Printf("%s", request)
        //fmt.Fprintf(stream, request)
		buf := make([]byte, 65536)
		for {
			conn.SetDeadline(time.Now().Add(2 * time.Second))
			n, err := conn.Read(buf)
			if err != nil {
            	conn.Write([]byte("bye"))
				fmt.Println("finish read")
				ch <- 1
				break
			}
			stream.Write(buf[0:n])	
			out_total += n
			fmt.Printf("%s", buf[0:n])
			//fmt.Printf("receive %d total %d\n", n, out_total)
		}
	}

	streamIn := func(stream net.Conn, conn *kcp.UDPSession, ch chan int) {
		reader := bufio.NewReader(stream)
		// buf := make([]byte, 65536)
        for {
			line, err := reader.ReadString('\n')
			if err != nil {
                conn.Write([]byte("bye"))
			    fmt.Println("finish upstream.")
			    ch <- 1
				break
			}
            conn.Write([]byte(line))
			//fmt.Print(line)
			
            //n, err := stream.Read(buf)
            //if err != nil {
            //	conn.Write([]byte("bye"))
			//	fmt.Println("finish upstream.")
			//	ch <- 1
			//	break
            //}
            //conn.Write(buf[0:n])
            // in_total += n
            // fmt.Printf("receive %d total %d\n", n, in_total)
        }
	}

	go streamIn(upstream, conn, ch)
	streamOut(upstream, conn, ch)
}

func main() {
	server()
}
