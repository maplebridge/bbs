#!/bin/sh
#°O¿ýipcs¸ê°T

IPCSCMD=`which ipcs`
BBSROOT="/var/maple"

date >> "${BBSROOT}""/run/ipcs.log"
${IPCSCMD} -a >> "${BBSROOT}""/run/ipcs.log"