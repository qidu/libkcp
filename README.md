# ***libqtp***
   
Please read ```qtproxy_client.c``` for library ```libqtp``` usage. 

## ***Build***
```shell
$mkdir build && cd build       
$cmake ../
$make
$ls -l build
```
## ***Demo***
start test server(golang)  
```shell
# test proxy
$cd build
$go get github.com/qidu/ktp-go
$go run ktproxy.go
$./qtproxy_client
# or test file
$cd build
$go run ../kcpserver.go
$./qtp_filetest
```
