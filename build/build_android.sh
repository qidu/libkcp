# -g -ggdb -O0
# -fsanitize=address -fsanitize=undefined -fsanitize=leak
# -fno-omit-frame-pointer
aarch64-linux-gnu-gcc -c -I../ ../ikcp.c ../qtpconnection.c
aarch64-linux-gnu-gcc-ar rcs libqtp_aarch64.a ikcp.o qtpconnection.o
aarch64-linux-gnu-gcc -I../ ../qtproxy_client.c ./libqtp_aarch64.a -o qtproxy_client_aarch64
aarch64-linux-gnu-gcc -I../ ../qtp_test.c ./libqtp_aarch64.a -o qtp_testfile_aarch64
