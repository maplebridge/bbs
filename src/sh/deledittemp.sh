#!/bin/sh
BBSROOT="/var/maple"

find "${BBSROOT}""/tmp" -ctime +24h -maxdepth 1 -type f -name '*.ori' -exec rm -f {} \;
find "${BBSROOT}""/tmp" -ctime +24h -maxdepth 1 -type f -name '*.tmp' -exec rm -f {} \;
