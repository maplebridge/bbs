#!/bin/sh
#####################
# Title: Rebuild BBS Code (Maple BBS)
# Note: Can only be run by user 'maple' or someone can 'sudo -u maple'
# Author: Bill Yuan-yao Shih <ckm.bbs@bbs.cs.nthu.edu.tw>
######################

SM="sudo -H -u maple"
MKCMD="gmake -f Makefile.new -j 9"
# MKCMD="make"
SUDO_PROMPT="Need to sudo to '%U', please enter password for '%u': "
# Sometimes it fails to locate svn correctly, so we set
# its full path here.  -- HBrian
# *** CHANGE IT IF MOVED TO NEW ENVIRONMENT ***
SVNCMD=`which svn`
BBSROOT="/var/maple"

if [ $USER = maple ] || [ $USER = root ]; then
	echo "You are '${USER}' so we don't need sudo"
	SM=""
fi

cd "${BBSROOT}""/src"

# Update source code from SVN
echo "Now update source code from SVN...."

echo "Running Subversion ('${SVNCMD}') ..."
${SM} ${SVNCMD} update

echo "Now rebuild source code..."

${SM} ${MKCMD} clean
# ${SM} ${MKCMD} freebsd && ${SM} ${MKCMD} install && ${SM} ${MKCMD} update 
${SM} ${MKCMD} freebsd
${SM} ${MKCMD} install 

# Copy showNUM to home of maple for MRTG to use
${SM} cp "${BBSROOT}""/bin/showNUM" "${BBSROOT}" && $SM chmod 755 "${BBSROOT}""/showNUM"
