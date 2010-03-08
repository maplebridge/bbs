#!/bin/sh

BBSUSER="maple"
BBSROOT="/var/maple"
IPCSCMD=`which ipcs`
IPCRMCMD=`which ipcrm`

# killbbs first
su ${BBSUSER} -c "${BBSROOT}""/src/sh/killbbs.sh"
sleep 2
su ${BBSUSER} -c "${BBSROOT}""/src/sh/ipcs.sh"
sleep 2
# stop inetd, so no one gets in
/etc/rc.d/inetd stop
su ${BBSUSER} -c "${BBSROOT}""/src/sh/ipcs.sh"
sleep 2
# killbbs again
su ${BBSUSER} -c "${BBSROOT}""/src/sh/killbbs.sh"
sleep 5
su ${BBSUSER} -c "${BBSROOT}""/src/sh/ipcs.sh"
sleep 2
# flush buffers
sync
sleep 5
sync
sleep 5
# kick everyone off
killall -u ${BBSUSER} bbsd
sleep 5
killall -u ${BBSUSER} bbsd
sleep 5
${IPCSCMD} -a
sleep 2
su ${BBSUSER} -c "${BBSROOT}""/src/sh/ipcs.sh"
sleep 2
# remove SHM
su ${BBSUSER} -c "${BBSROOT}""/src/sh/killbbs.sh"
sleep 2
${IPCSCMD} -a
sleep 2
su ${BBSUSER} -c "${BBSROOT}""/src/sh/ipcs.sh"
sleep 2
# rebuild maple
cd "${BBSROOT}"/src/
su ${BBSUSER} -c "${BBSROOT}""/src/sh/rebuild.sh"
sleep 75
${IPCSCMD} -a
sleep 2
su ${BBSUSER} -c "${BBSROOT}""/src/sh/ipcs.sh"
sleep 2
# clean BRDSHM
${IPCRMCMD} -M 2997
sleep 2
# clean UTMPSHM
${IPCRMCMD} -M 1998
sleep 2
# clean FILMSHM
${IPCRMCMD} -M 2999
sleep 2
# clean PIPSHM
${IPCRMCMD} -M 4998
sleep 2
# setup environment
${IPCSCMD} -a
sleep 2
su ${BBSUSER} -c "${BBSROOT}""/src/sh/ipcs.sh"
sleep 2
su ${BBSUSER} -c "${BBSROOT}""/bin/account"
sleep 120
su ${BBSUSER} -c "${BBSROOT}""/bin/camera"
sleep 5
${IPCSCMD} -a
sleep 2
su ${BBSUSER} -c "${BBSROOT}""/src/sh/ipcs.sh"
sleep 2
# start inetd
/etc/rc.d/inetd start
# /usr/sbin/inetd -wW -C 60
# if root has 1998, reboot
"${BBSROOT}""/src/sh/killroot1998.sh"
