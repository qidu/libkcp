nohup WORKMODE=recv go run ../kcpserver.go &
./kcp_test_nofec 127.0.0.1 send
sleep 2
nohup WORKMODE=send go run ../kcpserver.go &
./kcp_test_nofec 127.0.0.1 recv
