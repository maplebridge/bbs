#!/bin/sh
#======================================================
# Based on pttbbs (GPL v2) version:
#   $Id: newvers.sh,v 1.2 2003/06/22 14:45:17 in2 Exp $
# Author: Bill Yuan-yao Shih, a.k.a ckm 
#         <ckm@cs.nthu.edu.tw> Jan, 2008.
# This source is released in GPL v2 License.
#======================================================


# prevent localized logs
LC_ALL=C
export LC_ALL

t=`date`

# are we working in CVS?
if [ -d ".svn" ] ; then

    #determine branch
    branch=`svn info | grep '^URL: ' | sed 's/.*\/maplebbs\/\([a-zA-Z0-9_\-\.]*\)\/.*/\1/'`
    branch="$branch "

    #determine rev
    rev=`svn info | grep Revision | sed 's/Revision: /r/'`

    if [ "$rev" != "" ]
    then
   	t="$t, $branch$rev"
    fi

fi

cat << EOF > vers.c
    char    * const compile_time = "${t}";
EOF
