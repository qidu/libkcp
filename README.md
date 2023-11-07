# ***libqtp***
   
Please read ```qtproxy_client.c``` for library ```libqtp``` usage. 

## ***Build***
```shell
$mkdir build && cd build       
$cmake ../
$make
$
```
## ***Demo***
start test server(golang)  
```shell
$go get github.com/qidu/ktp-go
$go run ktproxy.go
$cd build
$./qtproxy_client
$
$go run ../kcpserver.go
$./qtp_filetest
```
