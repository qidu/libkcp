WORKMODE=recv nohup go run ../kcpserver.go &
./kcp_test_nofec 127.0.0.1 send && cat nohup.out
sleep 10
WORKMODE=send nohup go run ../kcpserver.go &
./kcp_test_nofec 127.0.0.1 recv
