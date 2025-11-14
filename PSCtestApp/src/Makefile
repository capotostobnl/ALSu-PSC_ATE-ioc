TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = PSCtest
# PSCtest.dbd will be created and installed
DBD += PSCtest.dbd

# PSCtest.dbd will be made up from these files:
PSCtest_DBD += base.dbd

# Include dbd files from all support applications:
#PSCtest_DBD += xxx.dbd
PSCtest_DBD += asyn.dbd
PSCtest_DBD += drvTester.dbd


# Add all the support libraries needed by this IOC
#PSCtest_LIBS += xxx
PSCtest_LIBS += asyn

PSCtest_SRCS += drvTester.cpp

# PSCtest_registerRecordDeviceDriver.cpp derives from PSCtest.dbd
PSCtest_SRCS += PSCtest_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
PSCtest_SRCS_DEFAULT += PSCtestMain.cpp
PSCtest_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#PSCtest_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
PSCtest_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

