#!/bin/sh
# 若share memory 被root佔了，則reboot

# for freebsd only
for i in `ipcs | grep root | awk '{print $3}'`
do
  if [ $i = 1998 ]; then
         reboot
  fi
done

# Linux 請用 ipcs 及 ipcrm shm
