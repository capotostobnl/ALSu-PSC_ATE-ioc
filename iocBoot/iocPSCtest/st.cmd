#!../../bin/linux-x86_64/PSCtest

#- You may have to change PSCtest to something else
#- everywhere it appears in this file

#< envPaths
putenv("EPICS_CA_ADDR_LIST", "127.0.0.1")
epicsEnvSet(EPICS_BASE,"/epics/base-7.0.9")

## Register all support components
dbLoadDatabase "../../dbd/PSCtest.dbd"
PSCtest_registerRecordDeviceDriver(pdbbase) 

## Load record instances
#dbLoadRecords("../../db/PSCtest.db","user=diag")
dbLoadRecords("../../db/Tester.db","P=PSCtest:","user=diag")

iocInit()

## Start any sequence programs
#seq sncPSCtest,"user=diag"

#dbpf PSCtest:IP:Settings-SP 10.0.142.47:5000
<<<<<<< HEAD
#dbpf PSCtest:IP:Settings-SP 10.69.26.2:5000
dbpf PSCtest:IP:Settings-SP 10.69.26.3:5000
=======
dbpf PSCtest:IP:Settings-SP 10.69.26.2:5000
>>>>>>> origin/dev
