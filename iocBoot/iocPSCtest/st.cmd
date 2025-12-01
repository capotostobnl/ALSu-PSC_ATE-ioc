#!../../bin/linux-x86_64/PSCtest

#- You may have to change PSCtest to something else
#- everywhere it appears in this file

#< envPaths
epicsEnvSet("EPICS_CA_ADDR_LIST", "127.0.0.1")
epicsEnvSet(EPICS_BASE,"/epics/base-7.0.9")

## Register all support components
dbLoadDatabase "../../dbd/PSCtest.dbd"
PSCtest_registerRecordDeviceDriver(pdbbase)

epicsEnvSet("IPPORT", "10.69.26.4:5000")

# ---------------------------
# Configure asyn tester driver
# ---------------------------
# Syntax:  TesterConfigure("PORT", "IP:Port")
# Make sure to specify the UDP port your tester uses (5555 normally)
TesterConfigure("TESTER1", "$(IPPORT)")

# ---------------------------
# Load all PV records
# ---------------------------
# IMPORTANT:
# The new DB requires BOTH P and PORT macros.
#   P    → PV prefix
#   PORT → asyn port name (must match TesterConfigure first arg)
dbLoadRecords("../../db/Tester.db", "P=PSCtest:,PORT=TESTER1,IPPORT=$(IPPORT)")

iocInit()


#asynReport 5, "TESTER1"

## Start any sequence programs
#seq sncPSCtest,"user=diag"

#dbpf PSCtest:IP:Settings-SP 10.0.142.47:5000
#dbpf PSCtest:IP:Settings-SP 10.69.26.2:5000
#dbpf PSCtest:IP:Settings-SP 10.69.26.3:5000
