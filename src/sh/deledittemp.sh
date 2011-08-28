#!/bin/sh
BBSROOT="/var/maple"

find "${BBSROOT}""/tmp" -ctime +24h -maxdepth 1 -type f -name '*.ori' -exec rm -f {} \;
find "${BBSROOT}""/tmp" -ctime +24h -maxdepth 1 -type f -name '*.tmp' -exec rm -f {} \;
find "${BBSROOT}""/tmp" -ctime +1h -maxdepth 1 -type f -name 'ctable.*' -exec rm -f {} \;
find "${BBSROOT}""/tmp" -ctime +1h -maxdepth 1 -type f -name 'ctablelist.*' -exec rm -f {} \;
find "${BBSROOT}""/tmp" -ctime +1h -maxdepth 1 -type f -name '*.dictd' -exec rm -f {} \;
