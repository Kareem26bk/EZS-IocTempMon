#!../../bin/linux-x86_64/myIoc

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/myIoc.dbd"
myIoc_registerRecordDeviceDriver pdbbase

## Load record instances (If we had .db files, they would go here)
# dbLoadRecords("db/myIoc.db","user=kareem")

cd "${TOP}/iocBoot/${IOC}"

## Start the EPICS IOC Core
iocInit

## =======================================================
## Start our custom Real-Time Temperature Monitor threads!
## =======================================================
startTempMonitor
