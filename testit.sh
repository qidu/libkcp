# test send from client
WORKMODE=recv go run ../kcpserver.go
./kcp_test_nofec 127.0.0.1 send

# test send from serv
WORKMODE=send go run ../kcpserver.go
./kcp_test_nofec 127.0.0.1 recv
