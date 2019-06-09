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
        printf("/**************************************/\n");
        printf("/*   Usage: recv_csi <target ip>  [option <label>]    */\n");
        printf("/**************************************/\n");
	return 0;
    }
    if (2 == argc){
    //    fp = fopen("data","a");
        //num = atoi(argv[1]);
	strcpy(ip, argv[1]);
	/*if(!isIp_v4(ip)){
		printf("IP invalid\n");
		return 0;
	}*/
	sprintf(url,"http://%s", ip);
       /* if (!fp){
            printf("Fail to open <output_file>, are you root?\n");
            fclose(fp);
            return 0;
        }*/   
    }
    if (3 == argc){
	strcpy(ip, argv[1]);
	sprintf(url,"http://%s", ip);
	type = 'l';
	label = atoi(argv[2]);
    }
    if (argc > 3){
        printf(" Too many input arguments !\n");
        return 0;
    }

    fd = open_csi_device();
    if (fd < 0){
        perror("Failed to open the device...");
        return errno;
    }
    
//    printf("#Receiving data! Press Ctrl+C to quit!\n");

    quit = 0;
    total_msg_cnt = 0;
    int fail = 0;
    while(fail < 10){
	
        if (1 == quit){
            return 0;
            fclose(fp);
            close_csi_device(fd);
        }

        /* keep listening to the kernel and waiting for the csi report */
        cnt = read_csi_buf(buf_addr,fd,BUFSIZE);

        if (cnt){
            total_msg_cnt += 1;

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
	     
	    if (csi_status->chanBW != 0 || csi_status->phyerr != 0 || csi_status->payload_len != 1056 || csi_status->channel != 2447 || csi_status->nr !=2 || csi_status->nc != 2 || csi_status->num_tones != 56)continue;
	    printf("__________\n");  
            printf("nr:%d,nc:%d,tones:%d,rate:%d,chanBW:%d,payload_len:%d,channel:%d\n",csi_status->nr,csi_status->nc,csi_status->num_tones,csi_status->rate,csi_status->chanBW,csi_status->payload_len,csi_status->channel);  
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
            printf("\n");
           // cont++;
		//printf("%d,%d\n",cont,num);
	    int i,j,k;
            if(cont==num)
            {
		printf("http_post\n");
		char message[2048] = {'\0'};
		char message2[2048] = {'\0'};
		time_t t;
		sprintf(message,"{\n  \"row\":4,\n  \"column\":56,\n  \"type\":\"%c\",\n  \"label\":%d,\n  \"time\":\"%d\",\n  \"csi_array\": [\n    ", type, label, time(NULL));
		
		for (i = 0; i < 2; i++){
        		for (j = 0; j < 2; j++){
        			for (k = 0; k < 56; k++){
					if (i == 1 && j == 1 && k == 55){
						//sprintf(message2,"%s%lf\n  ]\n}",message,(double)csi_matrix[i][j][k].real);
						sprintf(message2,"%lf\n  ]\n}",pow((double)csi_matrix[i][j][k].real,2)+ pow((double)csi_matrix[i][j][k].imag,2));
					}else{
						sprintf(message2,"%lf,",pow((double)csi_matrix[i][j][k].real,2)+ pow((double)csi_matrix[i][j][k].imag,2));
					}
					strcat(message,message2);
				
				}
			}
		}
		//printf("%s\n",message);
		char* result = http_post(url, message);
		printf("%s received.\n", result);
		//if (result == NULL) fail++;
		//else fail = 0;
		if (!result) free(result);
		//free(message);
               // quit=1;
            }
	   
        }
    }
    
    //fclose(fp);
    
    close_csi_device(fd);
    free(csi_status);
    return 0;
}
