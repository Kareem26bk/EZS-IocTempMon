# SPDX-FileCopyrightText: 2003 Argonne National Laboratory
#
# SPDX-License-Identifier: EPICS

TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = myIoc
# myIoc.dbd will be created and installed
DBD += myIoc.dbd

# myIoc.dbd will be made up from these files:
myIoc_DBD += base.dbd

# Include dbd files from all support applications:
#myIoc_DBD += xxx.dbd
myIoc_DBD += tempMonitor.dbd
# Add all the support libraries needed by this IOC
#myIoc_LIBS += xxx

# myIoc_registerRecordDeviceDriver.cpp derives from myIoc.dbd
myIoc_SRCS += myIoc_registerRecordDeviceDriver.cpp
myIoc_SRCS += tempMonitor.c

# Build the main IOC entry point on workstation OSs.
myIoc_SRCS_DEFAULT += myIocMain.cpp
myIoc_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#myIoc_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
myIoc_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
