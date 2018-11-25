CC_PATH=/home/ubuntu/corn/openwrt/staging_dir/toolchain-mips_24kc_gcc-7.3.0_musl/bin
CC_TOOL=mips-openwrt-linux-gcc-7.3.0

OBJS = csi_fun.o http.o main.o
CC = $(CC_PATH)/$(CC_TOOL)
recv_csi: $(OBJS)
	$(CC) $(OBJS) -o recv_csi
http.o: http.c http.h
	$(CC) -c http.c -o http.o
csi_fun.o: csi_fun.c csi_fun.h
	$(CC) -c csi_fun.c -o csi_fun.o
main.o: main.c csi_fun.h http.h
	$(CC) -c main.c -o main.o
clean: 
	rm -f *.o recv_csi

