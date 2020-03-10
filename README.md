For the purpose of cross-compile, please specify your path of CC_PATH and CC_TOOL in the makefile
```
CC_PATH=/home/ubuntu/corn/openwrt/staging_dir/toolchain-mips_24kc_gcc-7.3.0_musl/bin
CC_TOOL=mips-openwrt-linux-gcc-7.3.0
```
Send package to receiver
```
./send_data ifName DstMacAddr SendingRate(us)
Example: ./send_data wlan0 C4:E9:84:4E:BF:36 200
```
Receive normolized CSI
```
./recv_csi <target ip>  [-l -1][-t 200(ms)][-d]                                                    
      -l label: use specific label                  
      -t label: set interval for sending to server
      -d      : enable logging 
Example: ./recv_csi 192.168.2.2 -l 1 -t 300
```
Receive raw CSI (including complex part)
```
./recv_csi_raw <target ip>  [-l -1][-t 200(ms)][-d]                                                   
      -l label: use specific label                  
      -t label: set interval for sending to server
      -d      : enable logging
Example: ./recv_csi_raw 192.168.2.2 -l 1 -t 300
```
The user-space applications for our Atheros-CSI-TOOL

Please visit our maintainance page http://pdcc.ntu.edu.sg/wands/Atheros/ for detailed infomration.

If you want more details on how to use this tool, please request the documentation http://pdcc.ntu.edu.sg/wands/Atheros/install/install_info.html.

Change Log, we now support one transmitter multiple receivers at the same time. One packet transmitted can be simultaneously received by multiple receivers and the CSI will be calculated accordingly. More detail can be found from our maintainance page. http://pdcc.ntu.edu.sg/wands/Atheros/
