// drvTester.cpp - Unified PSC tester UDP driver (Tester + Tester2 as asynPortDriver)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <stdint.h>
#include <errno.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <epicsMutex.h>
#include <epicsThread.h>
#include <epicsExport.h>
#include <epicsTime.h>
#include <iocsh.h>
#include <asynPortDriver.h>

static void bytes_to_shorts(const uint8_t *byte_array,
                            uint16_t *short_array,
                            size_t num_bytes)
{
    for (size_t i = 0; i < num_bytes / 2; i++) {
        short_array[i] = (uint16_t)byte_array[2 * i]
                       | ((uint16_t)byte_array[2 * i + 1] << 8);
    }
}

// --------------------
// Parameter name strings
// --------------------

// Tester (original)
#define P_LastCmdString           "LAST_CMD"
#define P_LastCmdElapsedString    "LAST_CMD_ELAPSED"
#define P_IpPortString            "IP_PORT"
#define P_CALStateString          "CAL_STATE"
#define P_CH1ModeString           "CH1_MODE"
#define P_CH2ModeString           "CH2_MODE"
#define P_CH3ModeString           "CH3_MODE"
#define P_CH4ModeString           "CH4_MODE"
#define P_DCCTFaultChanString     "DCCT_FAULT_CH"
#define P_IgndChanString          "IGND_CH"
#define P_StatusString            "STATUS"

#define P_IgndString              "IGND"
#define P_Vgain1String            "VGAIN1"
#define P_Vgain2String            "VGAIN2"
#define P_Vgain3String            "VGAIN3"
#define P_Vgain4String            "VGAIN4"
#define P_Igain1String            "IGAIN1"
#define P_Igain2String            "IGAIN2"
#define P_Igain3String            "IGAIN3"
#define P_Igain4String            "IGAIN4"

#define P_P15V_14String           "P15V_14"
#define P_N15V_14String           "N15V_14"
#define P_P15V_58String           "P15V_58"
#define P_N15V_58String           "N15V_58"

// Manual command (Tester2)
#define P_ManualCmdString         "MANUAL_CMD"

// Tester2: DIxx, CALDAC, Polarity, PCFaults
#define P_CH1_FLT1String          "CH1_FLT1"
#define P_CH1_FLT2String          "CH1_FLT2"
#define P_CH1_SPAREString         "CH1_SPARE"

#define P_CH2_FLT1String          "CH2_FLT1"
#define P_CH2_FLT2String          "CH2_FLT2"
#define P_CH2_SPAREString         "CH2_SPARE"

#define P_CH3_FLT1String          "CH3_FLT1"
#define P_CH3_FLT2String          "CH3_FLT2"
#define P_CH3_SPAREString         "CH3_SPARE"

#define P_CH4_FLT1String          "CH4_FLT1"
#define P_CH4_FLT2String          "CH4_FLT2"
#define P_CH4_SPAREString         "CH4_SPARE"

#define P_CALDACString            "CALDAC"
#define P_PolarityString          "POLARITY"

#define P_PCFault1String          "PCFAULT1"
#define P_PCFault2String          "PCFAULT2"
#define P_PCFault3String          "PCFAULT3"
#define P_PCFault4String          "PCFAULT4"

// Forward decl
class TesterDriver;
static void testerPollThreadC(void* pPvt);

class TesterDriver : public asynPortDriver {
public:
    TesterDriver(const char* portName, const char* ipPort);
    ~TesterDriver();

    asynStatus readFloat64(asynUser* pasynUser, epicsFloat64* value) override;
    asynStatus writeInt32(asynUser* pasynUser, epicsInt32 value) override;
    asynStatus writeFloat64(asynUser* pasynUser, epicsFloat64 value) override;
    asynStatus writeOctet(asynUser* pasynUser,
                          const char* value,
                          size_t nChars,
                          size_t* nActual) override;

    void pollThread();

private:
    asynStatus sendCommand(const char* cmd,
                           uint16_t* replyShorts = nullptr,
                           size_t numShorts = 0);

    int sockfd_;
    struct sockaddr_in serverAddr_;
    epicsMutexId mutex_;


    // ---- Command Timestamping ---
    epicsTimeStamp lastCmdTime_;
    bool lastCmdValid_;
    // ---- Cache (Tester) ----
    int oldCAL_;
    int oldMode1_, oldMode2_, oldMode3_, oldMode4_;
    int oldDCCTfault_, oldIgndChan_;
    double oldIgnd_;
    double oldVgain1_, oldVgain2_, oldVgain3_, oldVgain4_;
    double oldIgain1_, oldIgain2_, oldIgain3_, oldIgain4_;

    // ---- Cache (Tester2) ----
    int oldCH1_F1_, oldCH1_F2_, oldCH1_S_;
    int oldCH2_F1_, oldCH2_F2_, oldCH2_S_;
    int oldCH3_F1_, oldCH3_F2_, oldCH3_S_;
    int oldCH4_F1_, oldCH4_F2_, oldCH4_S_;
    int oldPCFault1_, oldPCFault2_, oldPCFault3_, oldPCFault4_;
    int oldPolarity_;
    double oldCALDAC_;
    std::string oldManualCmd_;

    // Param indices
    int P_LastCmd;
    int P_LastCmdElapsed;
    int P_IpPort;
    int P_CALState;
    int P_CH1Mode;
    int P_CH2Mode;
    int P_CH3Mode;
    int P_CH4Mode;
    int P_DCCTFaultChan;
    int P_IgndChan;
    int P_Status;

    int P_Ignd;
    int P_Vgain1;
    int P_Vgain2;
    int P_Vgain3;
    int P_Vgain4;
    int P_Igain1;
    int P_Igain2;
    int P_Igain3;
    int P_Igain4;

    int P_P15V_14;
    int P_N15V_14;
    int P_P15V_58;
    int P_N15V_58;

    int P_ManualCmd;

    int P_CH1_FLT1;
    int P_CH1_FLT2;
    int P_CH1_SPARE;

    int P_CH2_FLT1;
    int P_CH2_FLT2;
    int P_CH2_SPARE;

    int P_CH3_FLT1;
    int P_CH3_FLT2;
    int P_CH3_SPARE;

    int P_CH4_FLT1;
    int P_CH4_FLT2;
    int P_CH4_SPARE;

    int P_CALDAC;
    int P_Polarity;

    int P_PCFault1;
    int P_PCFault2;
    int P_PCFault3;
    int P_PCFault4;
};

static void testerPollThreadC(void* pPvt)
{
    static_cast<TesterDriver*>(pPvt)->pollThread();
}

// --------------------
// Constructor
// --------------------
TesterDriver::TesterDriver(const char* portName, const char* ipPort)
  : asynPortDriver(portName,
                   1,
                   asynInt32Mask | asynFloat64Mask | asynOctetMask | asynDrvUserMask,
                   asynInt32Mask | asynFloat64Mask | asynOctetMask | asynDrvUserMask,
                   ASYN_CANBLOCK, 1, 0, 0),
    sockfd_(-1)
{

    // Create params (Tester)
    createParam(P_LastCmdString,      asynParamOctet,   &P_LastCmd);
    setStringParam(P_LastCmd, "N/A"); //Initialize to suppress warnings...
    createParam(P_LastCmdElapsedString,  asynParamFloat64,   &P_LastCmdElapsed);

    createParam(P_IpPortString,        asynParamOctet, &P_IpPort);
    createParam(P_CALStateString,      asynParamInt32,   &P_CALState);
    createParam(P_CH1ModeString,       asynParamInt32,   &P_CH1Mode);
    createParam(P_CH2ModeString,       asynParamInt32,   &P_CH2Mode);
    createParam(P_CH3ModeString,       asynParamInt32,   &P_CH3Mode);
    createParam(P_CH4ModeString,       asynParamInt32,   &P_CH4Mode);
    createParam(P_DCCTFaultChanString, asynParamInt32,   &P_DCCTFaultChan);
    createParam(P_IgndChanString,      asynParamInt32,   &P_IgndChan);
    createParam(P_StatusString,        asynParamInt32,   &P_Status);

    createParam(P_IgndString,          asynParamFloat64, &P_Ignd);
    createParam(P_Vgain1String,        asynParamFloat64, &P_Vgain1);
    createParam(P_Vgain2String,        asynParamFloat64, &P_Vgain2);
    createParam(P_Vgain3String,        asynParamFloat64, &P_Vgain3);
    createParam(P_Vgain4String,        asynParamFloat64, &P_Vgain4);
    createParam(P_Igain1String,        asynParamFloat64, &P_Igain1);
    createParam(P_Igain2String,        asynParamFloat64, &P_Igain2);
    createParam(P_Igain3String,        asynParamFloat64, &P_Igain3);
    createParam(P_Igain4String,        asynParamFloat64, &P_Igain4);

    createParam(P_P15V_14String,       asynParamFloat64, &P_P15V_14);
    createParam(P_N15V_14String,       asynParamFloat64, &P_N15V_14);
    createParam(P_P15V_58String,       asynParamFloat64, &P_P15V_58);
    createParam(P_N15V_58String,       asynParamFloat64, &P_N15V_58);

    createParam(P_ManualCmdString,     asynParamOctet,   &P_ManualCmd);

    // Tester2 params
    createParam(P_CH1_FLT1String,      asynParamInt32,   &P_CH1_FLT1);
    createParam(P_CH1_FLT2String,      asynParamInt32,   &P_CH1_FLT2);
    createParam(P_CH1_SPAREString,     asynParamInt32,   &P_CH1_SPARE);

    createParam(P_CH2_FLT1String,      asynParamInt32,   &P_CH2_FLT1);
    createParam(P_CH2_FLT2String,      asynParamInt32,   &P_CH2_FLT2);
    createParam(P_CH2_SPAREString,     asynParamInt32,   &P_CH2_SPARE);

    createParam(P_CH3_FLT1String,      asynParamInt32,   &P_CH3_FLT1);
    createParam(P_CH3_FLT2String,      asynParamInt32,   &P_CH3_FLT2);
    createParam(P_CH3_SPAREString,     asynParamInt32,   &P_CH3_SPARE);

    createParam(P_CH4_FLT1String,      asynParamInt32,   &P_CH4_FLT1);
    createParam(P_CH4_FLT2String,      asynParamInt32,   &P_CH4_FLT2);
    createParam(P_CH4_SPAREString,     asynParamInt32,   &P_CH4_SPARE);

    createParam(P_CALDACString,        asynParamFloat64, &P_CALDAC);
    createParam(P_PolarityString,      asynParamInt32,   &P_Polarity);

    createParam(P_PCFault1String,      asynParamInt32,   &P_PCFault1);
    createParam(P_PCFault2String,      asynParamInt32,   &P_PCFault2);
    createParam(P_PCFault3String,      asynParamInt32,   &P_PCFault3);
    createParam(P_PCFault4String,      asynParamInt32,   &P_PCFault4);

    // Init caches
    oldCAL_ = 0;
    oldMode1_ = oldMode2_ = oldMode3_ = oldMode4_ = 0;
    oldDCCTfault_ = 0;
    oldIgndChan_  = 0;
    oldIgnd_ = 0.0;
    oldVgain1_ = oldVgain2_ = oldVgain3_ = oldVgain4_ = 0.0;
    oldIgain1_ = oldIgain2_ = oldIgain3_ = oldIgain4_ = 0.0;

    oldCH1_F1_ = oldCH1_F2_ = oldCH1_S_ = 0;
    oldCH2_F1_ = oldCH2_F2_ = oldCH2_S_ = 0;
    oldCH3_F1_ = oldCH3_F2_ = oldCH3_S_ = 0;
    oldCH4_F1_ = oldCH4_F2_ = oldCH4_S_ = 0;

    oldPCFault1_ = oldPCFault2_ = oldPCFault3_ = oldPCFault4_ = 0;
    oldPolarity_ = 0;
    oldCALDAC_ = 0.0;
    oldManualCmd_.clear();

    mutex_ = epicsMutexCreate();


    lastCmdValid_ = false;
    epicsTimeGetCurrent(&lastCmdTime_);    // Initialize


    // Parse ipPort "A.B.C.D:PORT"
    char buf[128];
    strncpy(buf, ipPort, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';

    char* ip = strtok(buf, ":");
    char* portStr = strtok(NULL, ":");
    int port = portStr ? atoi(portStr) : 5555;
    printf("TesterDriver: configuring %s:%d\n", ip, port);
    fflush(stdout);

    memset(&serverAddr_, 0, sizeof(serverAddr_));
    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_port = htons(port);

    if (!ip || inet_pton(AF_INET, ip, &serverAddr_.sin_addr) <= 0) {
        printf("TesterDriver: invalid address '%s'\n", ipPort);
        return;
    }

    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) {
        perror("TesterDriver: socket creation failed");
        return;
    }

    struct timeval timeout;
    timeout.tv_sec  = 2;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO,
                   &timeout, sizeof(timeout)) < 0) {
        perror("TesterDriver: setsockopt RCVTIMEO failed");
    }
    if (setsockopt(sockfd_, SOL_SOCKET, SO_SNDTIMEO,
                   &timeout, sizeof(timeout)) < 0) {
        perror("TesterDriver: setsockopt SNDTIMEO failed");
    }

    epicsThreadCreate("testerPoll",
                      epicsThreadPriorityMedium,
                      epicsThreadGetStackSize(epicsThreadStackMedium),
                      testerPollThreadC, this);
}

TesterDriver::~TesterDriver()
{
    if (sockfd_ >= 0) {
        close(sockfd_);
    }
    if (mutex_) {
        epicsMutexDestroy(mutex_);
    }
}

asynStatus TesterDriver::sendCommand(const char* cmd,
                                     uint16_t* replyShorts,
                                     size_t numShorts)
{
    if (sockfd_ < 0) return asynError;

    epicsMutexLock(mutex_);
    printf("TesterDriver SEND: '%s'", cmd);
    fflush(stdout);

    ssize_t sent = sendto(sockfd_, cmd, strlen(cmd), 0,
                          (struct sockaddr*)&serverAddr_,
                          sizeof(serverAddr_));
    if (sent < 0) {
        perror("TesterDriver: sendto failed");
        epicsMutexUnlock(mutex_);
        return asynError;
    }
    if (strcmp(cmd, "D15?\n") != 0) {
    // Save last non-D15? command
    setStringParam(P_LastCmd, cmd);

    // Remember when it was sent
    epicsTimeGetCurrent(&lastCmdTime_);
    lastCmdValid_ = true;
    }


    if (replyShorts && numShorts > 0) {
        uint8_t buffer[1024];
        socklen_t len = sizeof(serverAddr_);
        int n = recvfrom(sockfd_, buffer, sizeof(buffer), 0,
                         (struct sockaddr*)&serverAddr_, &len);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                epicsMutexUnlock(mutex_);
                return asynTimeout;
            } else {
                perror("TesterDriver: recvfrom failed");
                epicsMutexUnlock(mutex_);
                return asynError;
            }
        }
        if ((size_t)n < 2 * numShorts) {
            epicsMutexUnlock(mutex_);
            return asynError;
        }
        bytes_to_shorts(buffer, replyShorts, 2 * numShorts);
    }

    epicsMutexUnlock(mutex_);
    return asynSuccess;
}

// --------------------
// writeInt32
// --------------------
asynStatus TesterDriver::writeInt32(asynUser* pasynUser, epicsInt32 value)
{
    int addr;
    getAddress(pasynUser, &addr);
    int function = pasynUser->reason;
    char cmd[64] = {0};

    if (function == P_CALState) {
        sprintf(cmd, "CAL%d\n", value);
        setIntegerParam(P_CALState, value);
    }
    else if (function == P_CH1Mode) {
        sprintf(cmd, "T1%d\n", value);
        setIntegerParam(P_CH1Mode, value);
    }
    else if (function == P_CH2Mode) {
        sprintf(cmd, "T2%d\n", value);
        setIntegerParam(P_CH2Mode, value);
    }
    else if (function == P_CH3Mode) {
        sprintf(cmd, "T3%d\n", value);
        setIntegerParam(P_CH3Mode, value);
    }
    else if (function == P_CH4Mode) {
        sprintf(cmd, "T4%d\n", value);
        setIntegerParam(P_CH4Mode, value);
    }
    else if (function == P_DCCTFaultChan) {
        sprintf(cmd, "D%d\n", value);
        setIntegerParam(P_DCCTFaultChan, value);
    }
    else if (function == P_IgndChan) {
        // full Ignd command sent in writeFloat64
        setIntegerParam(P_IgndChan, value);
    }
    // ---- Tester2 DIxx bits ----
    else if (function == P_CH1_FLT1) {
        sprintf(cmd, "DI11%d\n", value);
        setIntegerParam(P_CH1_FLT1, value);
    }
    else if (function == P_CH1_FLT2) {
        sprintf(cmd, "DI12%d\n", value);
        setIntegerParam(P_CH1_FLT2, value);
    }
    else if (function == P_CH1_SPARE) {
        sprintf(cmd, "DI13%d\n", value);
        setIntegerParam(P_CH1_SPARE, value);
    }
    else if (function == P_CH2_FLT1) {
        sprintf(cmd, "DI21%d\n", value);
        setIntegerParam(P_CH2_FLT1, value);
    }
    else if (function == P_CH2_FLT2) {
        sprintf(cmd, "DI22%d\n", value);
        setIntegerParam(P_CH2_FLT2, value);
    }
    else if (function == P_CH2_SPARE) {
        sprintf(cmd, "DI23%d\n", value);
        setIntegerParam(P_CH2_SPARE, value);
    }
    else if (function == P_CH3_FLT1) {
        sprintf(cmd, "DI31%d\n", value);
        setIntegerParam(P_CH3_FLT1, value);
    }
    else if (function == P_CH3_FLT2) {
        sprintf(cmd, "DI32%d\n", value);
        setIntegerParam(P_CH3_FLT2, value);
    }
    else if (function == P_CH3_SPARE) {
        sprintf(cmd, "DI33%d\n", value);
        setIntegerParam(P_CH3_SPARE, value);
    }
    else if (function == P_CH4_FLT1) {
        sprintf(cmd, "DI41%d\n", value);
        setIntegerParam(P_CH4_FLT1, value);
    }
    else if (function == P_CH4_FLT2) {
        sprintf(cmd, "DI42%d\n", value);
        setIntegerParam(P_CH4_FLT2, value);
    }
    else if (function == P_CH4_SPARE) {
        sprintf(cmd, "DI43%d\n", value);
        setIntegerParam(P_CH4_SPARE, value);
    }
    // Polarity
    else if (function == P_Polarity) {
        sprintf(cmd, "P%d\n", value);
        setIntegerParam(P_Polarity, value);
    }
    // PCFaults – send F1..F4 only on set (1)
    else if (function == P_PCFault1) {
        if (value == 1) sprintf(cmd, "F1\n");
        setIntegerParam(P_PCFault1, value);
    }
    else if (function == P_PCFault2) {
        if (value == 1) sprintf(cmd, "F2\n");
        setIntegerParam(P_PCFault2, value);
    }
    else if (function == P_PCFault3) {
        if (value == 1) sprintf(cmd, "F3\n");
        setIntegerParam(P_PCFault3, value);
    }
    else if (function == P_PCFault4) {
        if (value == 1) sprintf(cmd, "F4\n");
        setIntegerParam(P_PCFault4, value);
    }
    else {
        // generic
        setIntegerParam(function, value);
    }

    if (cmd[0] != '\0') {
        sendCommand(cmd);
    }

    callParamCallbacks(addr);
    return asynSuccess;
}

asynStatus TesterDriver::readFloat64(asynUser* pasynUser, epicsFloat64* value)
{
    int addr;
    getAddress(pasynUser, &addr);
    int function = pasynUser->reason;

    // Handle Int32-style params by upcasting to double
    if (function == P_CALState     ||
        function == P_CH1Mode      ||
        function == P_CH2Mode      ||
        function == P_CH3Mode      ||
        function == P_CH4Mode      ||
        function == P_DCCTFaultChan||
        function == P_IgndChan     ||
        function == P_Status       ||
        function == P_CH1_FLT1     ||
        function == P_CH1_FLT2     ||
        function == P_CH1_SPARE    ||
        function == P_CH2_FLT1     ||
        function == P_CH2_FLT2     ||
        function == P_CH2_SPARE    ||
        function == P_CH3_FLT1     ||
        function == P_CH3_FLT2     ||
        function == P_CH3_SPARE    ||
        function == P_CH4_FLT1     ||
        function == P_CH4_FLT2     ||
        function == P_CH4_SPARE    ||
        function == P_Polarity     ||
        function == P_PCFault1     ||
        function == P_PCFault2     ||
        function == P_PCFault3     ||
        function == P_PCFault4)
    {
        epicsInt32 ival = 0;
        getIntegerParam(addr, function, &ival);
        *value = static_cast<epicsFloat64>(ival);
        return asynSuccess;
    }

    // For real Float64 params, use the base implementation
    return asynPortDriver::readFloat64(pasynUser, value);
}


// --------------------
// writeFloat64
// --------------------
asynStatus TesterDriver::writeFloat64(asynUser* pasynUser, epicsFloat64 value)
{
    int addr;
    getAddress(pasynUser, &addr);
    int function = pasynUser->reason;
    char cmd[64] = {0};

    if (function == P_Ignd) {
        if (oldIgnd_ != value) {
            oldIgnd_ = value;
            int igndChan;
            getIntegerParam(P_IgndChan, &igndChan);
            sprintf(cmd, "Ignd%d%4.3f\n", igndChan, value);
        }
        setDoubleParam(P_Ignd, value);
    }
    else if (function == P_Vgain1) {
        if (oldVgain1_ != value) {
            oldVgain1_ = value;
            sprintf(cmd, "V1%4.3f\n", value);
        }
        setDoubleParam(P_Vgain1, value);
    }
    else if (function == P_Vgain2) {
        if (oldVgain2_ != value) {
            oldVgain2_ = value;
            sprintf(cmd, "V2%4.3f\n", value);
        }
        setDoubleParam(P_Vgain2, value);
    }
    else if (function == P_Vgain3) {
        if (oldVgain3_ != value) {
            oldVgain3_ = value;
            sprintf(cmd, "V3%4.3f\n", value);
        }
        setDoubleParam(P_Vgain3, value);
    }
    else if (function == P_Vgain4) {
        if (oldVgain4_ != value) {
            oldVgain4_ = value;
            sprintf(cmd, "V4%4.3f\n", value);
        }
        setDoubleParam(P_Vgain4, value);
    }
    else if (function == P_Igain1) {
        if (oldIgain1_ != value) {
            oldIgain1_ = value;
            sprintf(cmd, "I1%4.3f\n", value);
        }
        setDoubleParam(P_Igain1, value);
    }
    else if (function == P_Igain2) {
        if (oldIgain2_ != value) {
            oldIgain2_ = value;
            sprintf(cmd, "I2%4.3f\n", value);
        }
        setDoubleParam(P_Igain2, value);
    }
    else if (function == P_Igain3) {
        if (oldIgain3_ != value) {
            oldIgain3_ = value;
            sprintf(cmd, "I3%4.3f\n", value);
        }
        setDoubleParam(P_Igain3, value);
    }
    else if (function == P_Igain4) {
        if (oldIgain4_ != value) {
            oldIgain4_ = value;
            sprintf(cmd, "I4%4.3f\n", value);
        }
        setDoubleParam(P_Igain4, value);
    }
    else if (function == P_CALDAC) {
        if (oldCALDAC_ != value) {
            oldCALDAC_ = value;
            sprintf(cmd, "CALDAC%.6f\n", value);
        }
        setDoubleParam(P_CALDAC, value);
    }
    else {
        setDoubleParam(function, value);
    }

    if (cmd[0] != '\0') {
        sendCommand(cmd);
    }

    callParamCallbacks(addr);
    return asynSuccess;
}

// --------------------
// writeOctet (manual command + IP reconfig)
// --------------------
asynStatus TesterDriver::writeOctet(asynUser* pasynUser,
                                    const char* value,
                                    size_t nChars,
                                    size_t* nActual)
{
    int addr;
    getAddress(pasynUser, &addr);
    int function = pasynUser->reason;

    // --- New: handle IP/Port reconfiguration ---
    if (function == P_IpPort) {
        // value is "A.B.C.D:PORT"
        char buf[128];
        size_t len = (nChars < sizeof(buf)-1) ? nChars : sizeof(buf)-1;
        strncpy(buf, value, len);
        buf[len] = '\0';

        char* ip = strtok(buf, ":");
        char* portStr = strtok(NULL, ":");
        int port = portStr ? atoi(portStr) : 5555;

        epicsMutexLock(mutex_);

        // Close old socket if open
        if (sockfd_ >= 0) {
            close(sockfd_);
            sockfd_ = -1;
        }

        memset(&serverAddr_, 0, sizeof(serverAddr_));
        serverAddr_.sin_family = AF_INET;
        serverAddr_.sin_port   = htons(port);

        if (!ip || inet_pton(AF_INET, ip, &serverAddr_.sin_addr) <= 0) {
            printf("TesterDriver: invalid IP_PORT '%s'\n", value);
            epicsMutexUnlock(mutex_);
            *nActual = 0;
            return asynError;
        }

        sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd_ < 0) {
            perror("TesterDriver: socket creation failed in IP_PORT");
            epicsMutexUnlock(mutex_);
            *nActual = 0;
            return asynError;
        }

        struct timeval timeout;
        timeout.tv_sec  = 2;
        timeout.tv_usec = 0;
        if (setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO,
                       &timeout, sizeof(timeout)) < 0) {
            perror("TesterDriver: setsockopt RCVTIMEO failed (IP_PORT)");
        }
        if (setsockopt(sockfd_, SOL_SOCKET, SO_SNDTIMEO,
                       &timeout, sizeof(timeout)) < 0) {
            perror("TesterDriver: setsockopt SNDTIMEO failed (IP_PORT)");
        }

        epicsMutexUnlock(mutex_);

        // Store the string so it can be read back
        setStringParam(P_IpPort, value);
        *nActual = nChars;
    }
    // --- Existing manual command handling, now "else if" ---
    else if (function == P_ManualCmd) {
        char cmd[64] = {0};
        size_t len = (nChars < sizeof(cmd)-1) ? nChars : sizeof(cmd)-1;
        strncpy(cmd, value, len);
        cmd[len] = '\0';

        size_t slen = strlen(cmd);
        if (slen > 0 && cmd[slen-1] != '\n' && slen < sizeof(cmd)-2) {
            strcat(cmd, "\n");
        }

        oldManualCmd_ = cmd;
        sendCommand(cmd);
        setStringParam(P_ManualCmd, cmd);
        *nActual = nChars;
    } else {
        *nActual = 0;
    }

    callParamCallbacks(addr);
    return asynSuccess;
}


// --------------------
// Poll thread: D15?
// --------------------
void TesterDriver::pollThread()
{
    while (true) {
        uint16_t data[4] = {0};
        asynStatus st = sendCommand("D15?\n", data, 4);

        if (st == asynSuccess) {
            double p15_14 = (double)data[0] * 25.0 / 65536.0;
            double n15_14 = (double)data[1] * -25.0 / 65536.0;
            double p15_58 = (double)data[2] * 25.0 / 65536.0;
            double n15_58 = (double)data[3] * -25.0 / 65536.0;

            setDoubleParam(P_P15V_14, p15_14);
            setDoubleParam(P_N15V_14, n15_14);
            setDoubleParam(P_P15V_58, p15_58);
            setDoubleParam(P_N15V_58, n15_58);
            setIntegerParam(P_Status, 1);
        } else {
            setIntegerParam(P_Status, 0);
        }

        if (lastCmdValid_) {
            epicsTimeStamp now;
            epicsTimeGetCurrent(&now);

            double elapsed = epicsTimeDiffInSeconds(&now, &lastCmdTime_);

            setDoubleParam(P_LastCmdElapsed, elapsed);
        }

        callParamCallbacks();
        epicsThreadSleep(0.5);
    }
}

// --------------------
// IOC shell glue
// --------------------
extern "C" {

int TesterConfigure(const char* portName, const char* ipPort)
{
    new TesterDriver(portName, ipPort);
    return 0;
}

static const iocshArg TesterConfigureArg0 = {"portName", iocshArgString};
static const iocshArg TesterConfigureArg1 = {"ip:port",  iocshArgString};
static const iocshArg* const TesterConfigureArgs[] = {
    &TesterConfigureArg0,
    &TesterConfigureArg1
};
static const iocshFuncDef TesterConfigureFuncDef =
    {"TesterConfigure", 2, TesterConfigureArgs};

static void TesterConfigureCallFunc(const iocshArgBuf* args)
{
    TesterConfigure(args[0].sval, args[1].sval);
}

void TesterRegister(void)
{
    iocshRegister(&TesterConfigureFuncDef, TesterConfigureCallFunc);
}

epicsExportRegistrar(TesterRegister);

} // extern "C"
