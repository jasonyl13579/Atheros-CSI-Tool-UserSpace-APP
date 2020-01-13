CC_PATH=/home/corn/Atheros_CSI_tool_OpenWRT_src/staging_dir/toolchain-mips_24kc_gcc-7.3.0_musl/bin
CC_TOOL=mips-openwrt-linux-gcc-7.3.0
OBJS = csi_fun.o http.o main.o
OBJS2 = csi_fun.o http.o main_raw.o
CC = $(CC_PATH)/$(CC_TOOL)
recv_csi: $(OBJS)
	$(CC) $(OBJS) -o recv_csi
raw: $(OBJS2)
	$(CC) $(OBJS2) -o recv_csi_raw
http.o: http.c http.h
	$(CC) -c http.c -o http.o
csi_fun.o: csi_fun.c csi_fun.h
	$(CC) -c csi_fun.c -o csi_fun.o
main.o: main.c csi_fun.h http.h
	$(CC) -c main.c -o main.o
main_raw.o: main_raw.c csi_fun.h http.h
	$(CC) -c main_raw.c -o main_raw.o
clean: 
	rm -f *.o recv_csi
	rm recv_csi_raw

