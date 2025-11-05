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
#include "cadef.h"
#include <aSubRecord.h>
#include <registryFunction.h>
#include <epicsExport.h>

static int Tester2(aSubRecord *precord) {

    static int oldCH1=0,oldCH2=0,oldCH3=0,oldCH4=0;
    static float oldCAL=0;

    int CH1fault = *(int *)precord->a;
    int CH2fault = *(int *)precord->b;
    int CH3fault = *(int *)precord->c;
    int CH4fault = *(int *)precord->d;
    float CALdac = *(float *)precord->e;
    char cmd[20],val[10];

    if(oldCH1!=CH1fault){
        oldCH1 = CH1fault;
        strcpy(cmd,"DI1");
        sprintf(val,"%d",CH1fault);
        strcat(cmd,val);
        strcat(cmd,"\n");
//        printf("cmd =  %s\n",cmd);
    }else if(oldCH2!=CH2fault){
        oldCH2 = CH2fault;
        strcpy(cmd,"DI2");
        sprintf(val,"%d",CH2fault);
        strcat(cmd,val);
        strcat(cmd,"\n");
//        printf("cmd =  %s\n",cmd);
    }else if(oldCH3!=CH3fault){
        oldCH3 = CH3fault;
        strcpy(cmd,"DI3");
        sprintf(val,"%d",CH3fault);
        strcat(cmd,val);
        strcat(cmd,"\n");
//        printf("cmd =  %s\n",cmd);
    }else if(oldCH4!=CH4fault){
        oldCH4 = CH4fault;
        strcpy(cmd,"DI4");
        sprintf(val,"%d",CH4fault);
        strcat(cmd,val);
        strcat(cmd,"\n");
//        printf("cmd =  %s\n",cmd);
    }else if(oldCAL!=CALdac){
        oldCAL = CALdac;
        strcpy(cmd,"CALDAC");
        sprintf(val,"%.6f",CALdac);
        strcat(cmd,val);
        strcat(cmd,"\n");
//        printf("cmd =  %s\n",cmd);
    }

    memcpy((char *)precord->vala,cmd,strlen(cmd)*sizeof(char));

    return(0);
}
// Note the function must be registered at the end!
epicsRegisterFunction(Tester2);
