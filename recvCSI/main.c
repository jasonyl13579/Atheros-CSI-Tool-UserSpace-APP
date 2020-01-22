/*
 * =====================================================================================
 *       Filename:  main.c
 *
 *    Description:  Here is an example for receiving CSI matrix 
 *                  Basic CSi procesing fucntion is also implemented and called
 *                  Check csi_fun.c for detail of the processing function
 *        Version:  1.0
 *
 *         Author:  Yaxiong Xie
 *         Email :  <xieyaxiongfly@gmail.com>
 *   Organization:  WANDS group @ Nanyang Technological University
 *   
 *   Copyright (c)  WANDS group @ Nanyang Technological University
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include "http.h"
#include "csi_fun.h"
#include <time.h>
#define BUFSIZE 4096

int quit, num;
char ip[30]; 
char url[30];
unsigned char buf_addr[BUFSIZE];
unsigned char data_buf[1500];

COMPLEX csi_matrix[3][3][114];

csi_struct*   csi_status;
unsigned long long threshold = 200;
void sig_handler(int signo)
{
    if (signo == SIGINT)
        quit = 1;
}
int isIp_v4( char* ip){
        int num;
        int flag = 1;
        int counter=0;
        char* p = strtok(ip,".");

        while (p && flag ){
                num = atoi(p);

                if (num>=0 && num<=255 && (counter++<4)){
                        flag=1;
                        p=strtok(NULL,".");

                }
                else{
                        flag=0;
                        break;
                }
        }

        return flag && (counter==4);

}
int main(int argc, char* argv[])
{
    FILE*       fp;
    int         fd;
    int         i;
    int         total_msg_cnt,cnt;
    int         log_flag;
    unsigned char endian_flag;
    u_int16_t   buf_len;
    int cont=0;
    
    log_flag = 1;
    char type = 'n';
    int label = -1;
    csi_status = (csi_struct*)malloc(sizeof(csi_struct));
    /* check usage */
    if (1 == argc){
        /* If you want to log the CSI for off-line processing,
         * you need to specify the name of the output file
         */
        log_flag  = 0;
        printf("/******************************************************/\n");
        printf("/*   Usage: recv_csi <target ip>  [-l -1][-t 200(ms)] */\n");
	printf("/*                                                    */\n");
	printf("/*      -l label: use specific label                  */\n");
	printf("/*      -t label: set interval for sending to server  */\n");
        printf("/******************************************************/\n");
	return 0;
    }
    strcpy(ip, argv[1]);
    sprintf(url,"http://%s", ip); 
    if (3 == argc){
	label = atoi(argv[2]);
    }
    for(int i = 2; i < argc; i++) { 
	char *arg = argv[i];
	switch(arg[0]){
	    case '-': 
		switch(arg[1]) { 
		    case 'l': 
			if (i+1 < argc){
			    label = atoi(argv[i+1]);
			} 
			break; 
		    case 't': 
			if (i+1 < argc){
			    threshold = atoi(argv[i+1]) > 100 ? atoi(argv[i+1]) : 100;
			}	
			break; 
		    default: 
			printf("No such arguments !\n");
			break;
		}
            default: 
		break;
	}
    }
   
    /*if (argc > 3){
        printf(" Too many input arguments !\n");
        return 0;
    }*/

    fd = open_csi_device();
    if (fd < 0){
        perror("Failed to open the device...");
        return errno;
    }
    
//    printf("#Receiving data! Press Ctrl+C to quit!\n");

    quit = 0;
    total_msg_cnt = 0;
    int fail = 0;
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    unsigned long long past_time_in_mill = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000 ;
    while(fail < 10){
	
        if (1 == quit){
            return 0;
            fclose(fp);
            close_csi_device(fd);
        }

        /* keep listening to the kernel and waiting for the csi report */
	gettimeofday(&tv, NULL);
	unsigned long long now_time_in_mill = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000 ;
	//printf("Current time:%llu\n",now_time_in_mill);
	if ( now_time_in_mill - past_time_in_mill < threshold -10){
		if (now_time_in_mill - past_time_in_mill > 10)usleep((now_time_in_mill - past_time_in_mill)*200);
		continue;
	}
	//printf("Threhold time:%llu\n",now_time_in_mill);
        cnt = read_csi_buf(buf_addr,fd,BUFSIZE);

        if (cnt){
            //printf("test\n");

            /* fill the status struct with information about the rx packet */
            record_status(buf_addr, cnt, csi_status);

            /* 
             * fill the payload buffer with the payload
             * fill the CSI matrix with the extracted CSI value
             */
            record_csi_payload(buf_addr, csi_status, data_buf, csi_matrix); 
            
            /* Till now, we store the packet status in the struct csi_status 
             * store the packet payload in the data buffer
             * store the csi matrix in the csi buffer
             * with all those data, we can build our own processing function! 
             */
            //process_csi(data_buf, csi_status, csi_matrix); 
	     
	    if (csi_status->chanBW != 0 || csi_status->phyerr != 0 || csi_status->payload_len != 1056 )continue;
	    
	    printf("__________\n");  
            printf("nr:%d,nc:%d,tones:%d,rate:%d,chanBW:%d,payload_len:%d,channel:%d\n",csi_status->nr,csi_status->nc,csi_status->num_tones,csi_status->rate,csi_status->chanBW,csi_status->payload_len,csi_status->channel);  
	    if (csi_status->nr != 2 || csi_status->nc != 2)continue;
	    total_msg_cnt += 1;
        //    printf("Recv %dth msg with rate: 0x%02x | payload len: %d\n",total_msg_cnt,csi_status->rate,csi_status->payload_len);
            
            /* log the received data for off-line processing */
            if (log_flag){
               // printf("__________\n");
               /* for(int i=0;i<2;i++)
                    for(int j= 0;j<2;j++)
                        for(int k =0;k<56;k++)
                        {   
                            printf("%lf ",(double)csi_matrix[i][j][k].real);
                            printf("%lf\n",(double)csi_matrix[i][j][k].imag);
                        }*/
            //    printf("%d Record\n",cont+1);          
            }
	    int i,j,k;
	    int row = csi_status->nr * csi_status->nc;
	    int column = csi_status->num_tones;
		
		char message[3000] = {'\0'};
		char message2[3000] = {'\0'};
		time_t t;
		t = time(NULL);
		struct timeval  tv;
		gettimeofday(&tv, NULL);

		unsigned long long time_in_mill = 
			 (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000 ;
		//printf("%llu\n", time_in_mill);
		sprintf(message,"{\n  \"row\":%d,\n  \"column\":%d,\n  \"type\":\"%c\",\n  \"label\":%d,\n  \"time\":\"%llu\",\n  \"csi_array\": [\n    ", row, column, type, label, time_in_mill);
	
		for (i = 0; i < csi_status->nr; i++){
			for (j = 0; j < csi_status->nc; j++){
				for (k = 0; k < csi_status->num_tones; k++){
					if (i == 1 && j == 1 && k == csi_status->num_tones-1){
						//sprintf(message2,"%s%lf\n  ]\n}",message,(double)csi_matrix[i][j][k].real);
						sprintf(message2,"%.2lf\n  ]\n}",pow((double)csi_matrix[i][j][k].real,2)+ pow((double)csi_matrix[i][j][k].imag,2));
					}else{
						sprintf(message2,"%.2lf,",pow((double)csi_matrix[i][j][k].real,2)+ pow((double)csi_matrix[i][j][k].imag,2));
					}
					strcat(message,message2);
			
				}
			}
		}
		//printf("%s\n",message);
		char* result = http_post(url, message);
		past_time_in_mill = time_in_mill;
		printf("Post count:%d, Time:%llu\n",total_msg_cnt, time_in_mill);
		//printf("%s received.\n", result);
		//if (result == NULL) fail++;
		//else fail = 0;
		if (!result) free(result);
		//free(message);
	       // quit=1;
	}
	   
    }
    
    //fclose(fp);
    
    close_csi_device(fd);
    free(csi_status);
    return 0;
}
