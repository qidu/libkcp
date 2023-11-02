package main

import (
	"bytes"
	"fmt"
	"io"
	"log"
	"os"
	"time"

	"github.com/xtaci/kcp-go"
)

const port = ":9999"
func ListenTest() (*kcp.Listener, error) {
	usefec := os.Getenv("FEC")
	if usefec == "" {
    	    return kcp.ListenWithOptions(port, nil, 0, 0)
	}
        fmt.Println("init with fec(2,2)")
    	return kcp.ListenWithOptions(port, nil, 2, 2)
}

func server() {
	ch := make(chan int)
	l, err := ListenTest()
	if err != nil {
		panic(err)
	}
	l.SetDSCP(46)
	for {
		s, err := l.AcceptKCP()
		if err != nil {
			panic(err)
		}
		go handle_client(s, ch)
		for {
			select {
			case msg := <-ch:
				if msg != 0 {
					return
				}
			default:
			}
		}
	}
}
func handle_client(conn *kcp.UDPSession, ch chan int) {
	conn.SetWindowSize(1024, 1024)
	// conn.SetNoDelay(1, 20, 2, 1)
	conn.SetNoDelay(0, 40, 0, 0)
	conn.SetStreamMode(true)
	fmt.Println("new client", conn.RemoteAddr())
	count := 0
	total := 0
	writed := 0

	workmode := os.Getenv("WORKMODE")
	if workmode == "" {
		workmode = string("send")
	}
	if bytes.Equal([]byte(workmode), []byte("send")) { // work in send mode
		file, err := os.Open("/tmp/filename")
		if err != nil {
			log.Fatal(err)
		}
		defer file.Close()
		readbuf := make([]byte, 1000)
		n, err := conn.Read(readbuf)
		if err != nil {
			panic(err)
		}
		fmt.Println(string(readbuf))
		if bytes.Equal(readbuf[:8], []byte("/tmp/filename")[:8]) {
			fmt.Println("Get file request")
		}
		readbuf = make([]byte, 20000)
		for {
			count++
			n, err = file.Read(readbuf)
			if err == io.EOF {
				break
			}
			if err != nil {
			}
			writed, err = conn.Write(readbuf[0:n])
			if err != nil {
				continue
			}
			total += writed
			fmt.Printf("count %d read %d write %d total %d\n", count, n, writed, total)
		}
		conn.Write([]byte("bye"))
                for {
		    n, err = conn.Read(readbuf)
		    if err == io.EOF {
                        break
                    }
		    if err != nil {
		    }
                    if n == 3 && bytes.Equal(readbuf[0:3], []byte("bye")) {
		        ch <- 1
		        fmt.Println("finish sending file.")
                        break
                    }
		    time.Sleep(1 * time.Second)
                }
	} else {
		buf := make([]byte, 65536)
		for {
			count++
			n, err := conn.Read(buf)
			if err != nil {
				panic(err)
			}
			total += n
			fmt.Printf("count %d receive %d total %d\n", count, n, total)
			if bytes.Equal(buf[n-3:n], []byte("bye")) {
				ch <- 1
				fmt.Println("finish recving file.")
				break
			}
		}
	}
}

func main() {
	fmt.Println("WORKMODE option: [send | recv]. default value is \"send\".")
	workmode := os.Getenv("WORKMODE")
	if workmode == "" {
		workmode = string("send")
	}
	fmt.Println("Current mode:", workmode)
	server()
}
