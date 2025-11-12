#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include "cadef.h"
#include <aSubRecord.h>
#include <registryFunction.h>
#include <epicsExport.h>

//#define IPADDR "10.0.142.XXX"
//#define PORT 5555
#define BUFFER_SIZE 1024

void bytes_to_shorts(const uint8_t *byte_array, uint16_t *short_array, size_t num_bytes) {
    for (size_t i = 0; i < num_bytes / 2; i++) {
        // Combine two bytes into a uint16_t (little-endian)
        short_array[i] = (uint16_t)byte_array[2 * i] | ((uint16_t)byte_array[2 * i + 1] << 8);
    }
}

short* UDPcommand(char *command, char *IPset){
//    printf("In UDPcommand with IPset = %s\n",IPset);
    char *IPADDR = strtok(IPset,":");
    int PORT = atoi(strtok(NULL,":"));
    printf("%s",command);

    int sockfd;
    unsigned char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    socklen_t len = sizeof(server_addr);
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return(NULL);
    }
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt failed");
        close(sockfd);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt send timeout failed");
        close(sockfd);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, IPADDR, &server_addr.sin_addr) <= 0) {
        perror("invalid address/ address not supported");
        close(sockfd);
        return(NULL);
    }
//    printf("Socket Created OK.\n");
    ssize_t bytes_sent = sendto(sockfd, command, strlen(command), 0, (const struct sockaddr *) &server_addr, sizeof(server_addr));
    if(bytes_sent < 0) {
        perror("sendto failed");
        close(sockfd);
        return(NULL);
    }
//    printf("UDP Command Sent OK.\n");
    if(strcmp(command,"D15?\n")==0){
//        printf("Setting up to read UDP data...\n");
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &server_addr, &len);
        buffer[n] = '\0';
//        printf("n read = %d\n",n);
        if(n<0){
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
//                printf("UDP receive timed out!\n");
            } else {
                perror("sendto failed");
            }
            close(sockfd);
            return(NULL);
        }
        short* statusArray = (short*)malloc(4*sizeof(short));
        bytes_to_shorts(buffer, statusArray, 8);
        close(sockfd);
        return(statusArray);
    }
    close(sockfd);
    return(NULL);
}

static int Tester(aSubRecord *precord) {

    static int oldCAL=0;
    static int oldMode1=0,oldMode2=0,oldMode3=0,oldMode4=0;
    static int oldDCCTfault=0,oldIgndChan=0;
    static float oldIgnd=0,oldVgain1=0,oldVgain2=0,oldVgain3=0;
    static float oldVgain4=0;
    static float oldIgain4=0,oldIgain1=0,oldIgain2=0,oldIgain3=0;
    static char oldTester[30];
    char *IPset = (char *)precord->a;
    int status;

    int CALstate = *(int *)precord->e;
    int CH1mode = *(int *)precord->f;
    int CH2mode = *(int *)precord->g;
    int CH3mode = *(int *)precord->h;
    int CH4mode = *(int *)precord->i;
    int DCCTfault = *(int *)precord->j;
    int IgndChan = *(int *)precord->k;
    float Ignd = *(float *)precord->l;
    float Vgain1 = *(float *)precord->m;
    float Vgain2 = *(float *)precord->n;
    float Vgain3 = *(float *)precord->o;
    float Vgain4 = *(float *)precord->p;
    float Igain1 = *(float *)precord->q;
    float Igain2 = *(float *)precord->r;
    float Igain3 = *(float *)precord->s;
    float Igain4 = *(float *)precord->t;
    char *TesterCMD = (char *)precord->u;
    char cmd[20], val[10];

    if(oldCAL!=CALstate){
        oldCAL = CALstate;
        strcpy(cmd,"CAL");
        sprintf(val,"%d",CALstate);
        strcat(cmd,val);
        strcat(cmd,"\n");
        UDPcommand(cmd,IPset);
    }else if(oldMode1!=CH1mode){
        oldMode1 = CH1mode;
        strcpy(cmd,"T1");
        sprintf(val,"%d",CH1mode);
        strcat(cmd,val);
        strcat(cmd,"\n");
        UDPcommand(cmd,IPset);
    }else if(oldMode2!=CH2mode){
        oldMode2 = CH2mode;
        strcpy(cmd,"T2");
        sprintf(val,"%d",CH2mode);
        strcat(cmd,val);
        strcat(cmd,"\n");
        UDPcommand(cmd,IPset);
    }else if(oldMode3!=CH3mode){
        oldMode3 = CH3mode;
        strcpy(cmd,"T3");
        sprintf(val,"%d",CH3mode);
        strcat(cmd,val);
        strcat(cmd,"\n");
        UDPcommand(cmd,IPset);
    }else if(oldMode4!=CH4mode){
        oldMode4 = CH4mode;
        strcpy(cmd,"T4");
        sprintf(val,"%d",CH4mode);
        strcat(cmd,val);
        strcat(cmd,"\n");
        UDPcommand(cmd,IPset);
    }else if(oldDCCTfault!=DCCTfault){
        oldDCCTfault = DCCTfault;
        strcpy(cmd,"D");
        sprintf(val,"%d",DCCTfault);
        strcat(cmd,val);
        strcat(cmd,"\n");
        UDPcommand(cmd,IPset);
    }else if(oldIgndChan!=IgndChan || oldIgnd!=Ignd){
        oldIgndChan = IgndChan;
        oldIgnd = Ignd;
        strcpy(cmd,"Ignd");
        sprintf(val,"%d",IgndChan);
        strcat(cmd,val);
        sprintf(val,"%4.3f",Ignd);
        strcat(cmd,val);
        strcat(cmd,"\n");
        printf("%s\n",cmd);
        UDPcommand(cmd,IPset);
    }else if(oldVgain1!=Vgain1){
        oldVgain1 = Vgain1;
        strcpy(cmd,"V1");
        sprintf(val,"%4.3f",Vgain1);
        strcat(cmd,val);
        strcat(cmd,"\n");
        UDPcommand(cmd,IPset);
    }else if(oldVgain2!=Vgain2){
        oldVgain2 = Vgain2;
        strcpy(cmd,"V2");
        sprintf(val,"%4.3f",Vgain2);
        strcat(cmd,val);
        strcat(cmd,"\n");
        UDPcommand(cmd,IPset);
    }else if(oldVgain3!=Vgain3){
        oldVgain3 = Vgain3;
        strcpy(cmd,"V3");
        sprintf(val,"%4.3f",Vgain3);
        strcat(cmd,val);
        strcat(cmd,"\n");
        UDPcommand(cmd,IPset);
    }else if(oldVgain4!=Vgain4){
        oldVgain4 = Vgain4;
        strcpy(cmd,"V4");
        sprintf(val,"%4.3f",Vgain4);
        strcat(cmd,val);
        strcat(cmd,"\n");
        UDPcommand(cmd,IPset);
    }else if(oldIgain1!=Igain1){
        oldIgain1 = Igain1;
        strcpy(cmd,"I1");
        sprintf(val,"%4.3f",Igain1);
        strcat(cmd,val);
        strcat(cmd,"\n");
        UDPcommand(cmd,IPset);
    }else if(oldIgain2!=Igain2){
        oldIgain2 = Igain2;
        strcpy(cmd,"I2");
        sprintf(val,"%4.3f",Igain2);
        strcat(cmd,val);
        strcat(cmd,"\n");
        UDPcommand(cmd,IPset);
    }else if(oldIgain3!=Igain3){
        oldIgain3 = Igain3;
        strcpy(cmd,"I3");
        sprintf(val,"%4.3f",Igain3);
        strcat(cmd,val);
        strcat(cmd,"\n");
        UDPcommand(cmd,IPset);
    }else if(oldIgain4!=Igain4){
        oldIgain4 = Igain4;
        strcpy(cmd,"I4");
        sprintf(val,"%4.3f",Igain4);
        strcat(cmd,val);
        strcat(cmd,"\n");
        UDPcommand(cmd,IPset);
    }else if(strcmp(oldTester,TesterCMD)!=0){
//        printf("New Tester2 CMD: %s   %s\n",TesterCMD,oldTester);
        strcpy(oldTester,TesterCMD);
        UDPcommand(TesterCMD,IPset);
    }else{
//        printf("Sending D15? command.\n");
        strcpy(cmd,"D15?\n");
        uint16_t *data = UDPcommand(cmd,IPset);
//        printf("Returning from D15? command.\n");
        if(data != NULL){
            status = 1;
            *(float *)precord->vala = (float)data[0]*25.0/65536.0;
            *(float *)precord->valb = (float)data[1]*-25.0/65536.0;
            *(float *)precord->valc = (float)data[2]*25.0/65536.0;
            *(float *)precord->vald = (float)data[3]*-25.0/65536.0;
            *(int *) precord->vale = status;
            free(data);
        }else{
            status = 0;
            *(int *) precord->vale = status;
            printf("Array is NULL\n");
            free(data);
        }
    }
    return(0);
}
// Note the function must be registered at the end!
epicsRegisterFunction(Tester);
