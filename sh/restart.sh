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
/usr/bin/ipcs -a
# remove SHM
/usr/home/maple/src/sh/killbbs.sh
/usr/bin/ipcs -a
sleep 5
# rebuild maple
cd /usr/home/maple/src/
su maple -c /usr/home/maple/src/sh/rebuild.sh
/usr/bin/ipcs -a
sleep 75
# clean BRDSHM
/usr/bin/ipcrm -M 2997
sleep 2
# clean UTMPSHM
/usr/bin/ipcrm -M 1998
sleep 2
# clean FILMSHM
/usr/bin/ipcrm -M 2999
sleep 2
# clean PIPSHM
/usr/bin/ipcrm -M 4998
sleep 2
# setup environment
su maple -c /usr/home/maple/bin/camera
sleep 5
su maple -c /usr/home/maple/bin/account
sleep 120
# start inetd
/etc/rc.d/inetd start
#/usr/sbin/inetd -wW -C 60
