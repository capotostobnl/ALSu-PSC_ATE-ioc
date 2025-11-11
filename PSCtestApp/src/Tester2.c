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

    static int oldCH1_F1=0, oldCH1_F2=0, oldCH1_S=0;
    static int oldCH2_F1=0, oldCH2_F2=0, oldCH2_S=0;
    static int oldCH3_F1=0, oldCH3_F2=0, oldCH3_S=0;
    static int oldCH4_F1=0, oldCH4_F2=0, oldCH4_S=0;
    static float oldCAL=0;
    static char oldManCmd[20];
    static int oldPolarity=0;

    int CH1_F1 = *(int *)precord->a; // INPA
    int CH1_F2 = *(int *)precord->b; // INPB
    int CH1_S  = *(int *)precord->c; // INPC
    
    int CH2_F1 = *(int *)precord->d; // INPD
    int CH2_F2 = *(int *)precord->e; // INPE
    int CH2_S  = *(int *)precord->f; // INPF
    
    int CH3_F1 = *(int *)precord->g; // INPG
    int CH3_F2 = *(int *)precord->h; // INPH
    int CH3_S  = *(int *)precord->i; // INPI
    
    int CH4_F1 = *(int *)precord->j; // INPJ
    int CH4_F2 = *(int *)precord->k; // INPK
    int CH4_S  = *(int *)precord->l; // INPL
    
    float CALdac = *(float *)precord->m; // INPM

    char *ManCmd = (char *)precord->n;    // INPN

    int Polarity = *(int *)precord->o;    // INPO

    char cmd[20] = ""; // Local buffer

    if(strcmp(oldManCmd,ManCmd)!=0){
        strcpy(oldManCmd,ManCmd);
        
        if (strlen(ManCmd) > 0) {
            int len;
            // Copy up to 19 chars (from 20-char input)
            strncpy(cmd, ManCmd, 19);
            // Terminate at index 19 (the 20th char)
            cmd[19] = '\0';
            
            len = strlen(cmd);

            // Check for newline
            if (len > 0 && cmd[len-1] != '\n') {
                // Check for space (18 leaves room for \n and \0)
                if (len < 18) {
                    strcat(cmd, "\n"); 
                }
            }
        }
        strcpy(ManCmd, ""); // Consume
    }
    else if(oldCH1_F1 != CH1_F1){
        oldCH1_F1 = CH1_F1;
        sprintf(cmd, "DI11%d\n", CH1_F1); // DI[ch][bit][state]
    }
    else if(oldCH1_F2 != CH1_F2){
        oldCH1_F2 = CH1_F2;
        sprintf(cmd, "DI12%d\n", CH1_F2); 
    }
    else if(oldCH1_S != CH1_S){
        oldCH1_S = CH1_S;
        sprintf(cmd, "DI13%d\n", CH1_S);
    }
    else if(oldCH2_F1 != CH2_F1){
        oldCH2_F1 = CH2_F1;
        sprintf(cmd, "DI21%d\n", CH2_F1);
    }
    else if(oldCH2_F2 != CH2_F2){
        oldCH2_F2 = CH2_F2;
        sprintf(cmd, "DI22%d\n", CH2_F2);
    }
    else if(oldCH2_S != CH2_S){
        oldCH2_S = CH2_S;
        sprintf(cmd, "DI23%d\n", CH2_S);
    }
    else if(oldCH3_F1 != CH3_F1){
        oldCH3_F1 = CH3_F1;
        sprintf(cmd, "DI31%d\n", CH3_F1);
    }
    else if(oldCH3_F2 != CH3_F2){
        oldCH3_F2 = CH3_F2;
        sprintf(cmd, "DI32%d\n", CH3_F2);
    }
    else if(oldCH3_S != CH3_S){
        oldCH3_S = CH3_S;
        sprintf(cmd, "DI33%d\n", CH3_S);
    }
    else if(oldCH4_F1 != CH4_F1){
        oldCH4_F1 = CH4_F1;
        sprintf(cmd, "DI41%d\n", CH4_F1);
    }
    else if(oldCH4_F2 != CH4_F2){
        oldCH4_F2 = CH4_F2;
        sprintf(cmd, "DI42%d\n", CH4_F2);
    }
    else if(oldCH4_S != CH4_S){
        oldCH4_S = CH4_S;
        sprintf(cmd, "DI43%d\n", CH4_S);
    }
    else if(oldCAL != CALdac){
        oldCAL = CALdac;
        sprintf(cmd, "CALDAC%.6f\n", CALdac);
    }
    else if(oldPolarity != Polarity){
        oldPolarity = Polarity;
        sprintf(cmd, "P%d\n", Polarity);
    }
    else if(oldBPCFault1 != BPCFault1){
        oldBPCFault1 = BPCFault1;
        if (BPCFault1 == 1) sprintf(cmd, "F1\n"); // CH1 Set
        else sprintf(cmd, "\n"); // CH1 Reset (Placeholder)
    }
    else if(oldBPCFault2 != BPCFault2){
        oldBPCFault2 = BPCFault2;
        if (BPCFault2 == 1) sprintf(cmd, "F2\n"); // CH2 Set
        else sprintf(cmd, "\n"); // CH2 Reset (Placeholder)
    }
    else if(oldBPCFault3 != BPCFault3){
        oldBPCFault3 = BPCFault3;
        if (BPCFault3 == 1) sprintf(cmd, "F3\n"); // CH3 Set
        else sprintf(cmd, "\n"); // CH3 Reset (Placeholder)
    }
    else if(oldBPCFault4 != BPCFault4){
        oldBPCFault4 = BPCFault4;
        if (BPCFault4 == 1) sprintf(cmd, "F4\n"); // CH4 Set
        else sprintf(cmd, "\n"); // CH4 Reset (Placeholder)
    }

    // Copy command to output if one was built
    if (strlen(cmd) > 0) {
    // Use strncpy to copy AND clear the rest of the buffer
    strncpy((char *)precord->vala, cmd, 30); 
    
    // As a safety, manually null-terminate the very last byte
    ((char *)precord->vala)[29] = '\0';
    }

    return(0);
}
// Note the function must be registered at the end!
epicsRegisterFunction(Tester2);