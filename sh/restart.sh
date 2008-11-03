#!/bin/sh

# stop inetd, so no one gets in
/etc/rc.d/inetd stop
sleep 5
# flush buffers
sync
sleep 5
sync
sleep 5
# kick everyone off
killall -u maple bbsd
sleep 5
killall -u maple bbsd
sleep 5
# remove SHM
/usr/home/maple/src/sh/killbbs.sh
sleep 5
# rebuild maple
cd /usr/home/maple/src/
su maple -c /usr/home/maple/src/sh/rebuild.sh
sleep 75
# setup environment
su maple -c /usr/home/maple/bin/camera
sleep 5
su maple -c /usr/home/maple/bin/account
sleep 120
# start inetd
/etc/rc.d/inetd start
#/usr/sbin/inetd -wW -C 60
