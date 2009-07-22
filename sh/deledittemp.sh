#!/bin/sh
find /var/maple/tmp -ctime +24h -maxdepth 1 -type f -name '*.ori' -exec rm -f {} \;
find /var/maple/tmp -ctime +24h -maxdepth 1 -type f -name '*.tmp' -exec rm -f {} \;
