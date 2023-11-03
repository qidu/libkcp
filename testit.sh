# create test data file
dd if=/dev/zero of=/tmp/filename bs=1M count=2

# test send from client
WORKMODE=recv go run ../kcpserver.go
./qtp_test 127.0.0.1 send

# test send from serv
WORKMODE=send go run ../kcpserver.go
./qtp_test 127.0.0.1 recv
