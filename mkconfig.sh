#! /bin/ksh
#  -*- Mode: Shell-script -*- 
# ----------------------------------------------------------------------
# Time-stamp:        "2004-11-01 11:58:32 bkorb"
# Author:            Bruce Korb <bkorb@gnu.org>
# Maintainer:        Bruce Korb <bkorb@gnu.org>
# Created:           Fri Jul 30 10:57:13 1999			      
#            by: bkorb
# ----------------------------------------------------------------------
# @(#) $Id: mkconfig.sh,v 3.12 2004/11/02 04:03:59 bkorb Exp $
# ----------------------------------------------------------------------
case "$1" in
-CVS ) update_cvs=true  ;;
*    ) update_cvs=false ;;
esac

sd=$(\cd $(dirname $0) > /dev/null 2>&1 && pwd)
if [ -z "$sd" ]
then
  echo "Cannot locate source directory for $0" >&2
  exit 1
fi

tmpag=$( (mktemp -d $HOME/tmp/ZZXXXXXX.d) 2>/dev/null)
test -z "${tmpag}" && tmpag=$HOME/tmp/ZZ$$.d && mkdir -p ${tmpag}

cd ${tmpag} || exit 1
cfgfile=$(\cd .. >/dev/null && pwd)/config$$.tmp
exec 5> ${cfgfile}
cp -frp ${sd}/* .

find * -type f | xargs chmod u+w
find * -type d | xargs chmod 755

bstr=$(set -- c*f*g/bootstrap ; echo $1)
sh $bstr || {
  print -u2 "error:  bootstrap failed"
  exit 1
}

nl='
'
GENLIST=''
for f in `find * -type f | sort`
do
  case "$f" in
  */stamp*    | \
  configure   | \
  config*.tmp | \
  *.in        | \
  *~          | \
  aclocal*    | \
  autom4*     ) : ;;
  * )
    [ -f ${sd}/${f} ] || GENLIST="${GENLIST}${f}${nl}"
    ;;
  esac
done

touch_list="`egrep '## stamp-.*GEN-RULE' agen5/Makefile.am | \
             sed 's@.*## *\(.*\) GEN-RULE@agen5/\1@' `"

#  Make sure the new file removes the current collection of files
#
cat <<'_EOF_' >&5
#! /bin/sh
#
#  DO NOT EDIT THIS FILE
#
#  It is generated by mkconfig.sh.  Make necessary changes there.
#		      
# ----------------------------------------------------------------------

SVDIR=`pwd`
cd `echo $0 | sed 's;/[^/]*$;;'`
SRCDIR=`pwd`

if [ ! -f ./configure.in ]
then
  echo cannot locate top source directory for AutoGen
  exit 1
fi

if [ -z "${VERBOSE}" ]
then VERBOSE=false ; VERBOSE_ARG=""
else VERBOSE=true ; VERBOSE_ARG="-x" ; set -x ; fi
export VERBOSE VERBOSE_ARG

#  Touch the stamp-* files so bootstrap.dir won't try to build them
#
_EOF_

cat >&5 <<_EOF_
touch -t 200001010000 $(echo ${touch_list})

_EOF_

for f in ${GENLIST}
do echo $f ; done | \
columns --spread=1 -I10 --first='for f in ' --line=" \\" >&5

cat <<'_EOF_' >&5
do
  if [ -f $f ]
  then
    rm -f $f || {
      echo cannot remove $f
      exit 1
    }
  fi
done

( uudecode -o /dev/stdout | \
  gunzip | \
_EOF_
print -u5 '  tar -xvf - ) <<_EOArchive_'

for f in ${GENLIST}
do

  sed \
    -e '/DO NOT EDIT THIS FILE/,/\(and the template file\|It has been extracted by getdefs\)/d' \
    ${f} > ${f}.XX && \
  mv -f ${f}.XX ${f}
  touch -r ${sd}/${f}
done

tar cf - ${GENLIST} | \
  gzip --best | \
  uuencode -m /dev/stdout >&5

print -u5 '_EOArchive_'

#  Finally, add the code that generates the files generated
#  by standard tools (yacc/byacc/bison).
#
cat <<'_EOF_' >&5

sh ${VERBOSE_ARG} config/bootstrap $@ || exit 1
cd $SVDIR
sh ${VERBOSE_ARG} ${SRCDIR}/configure $@
exit 0
# noag-boot.sh ends here
_EOF_

exec 5>&-

if cmp ${cfgfile} ${sd}/noag-boot.sh
then
  echo noag-boot.sh is unchanged
  rm -f ${cfgfile}

elif ${update_cvs:-false}
then
  cd $sd
  cvs edit noag-boot.sh
  mv -f ${cfgfile} noag-boot.sh
  cvs commit -m'CVS-ed noag-boot.sh script update' noag-boot.sh

else
  ( cd $sd
    echo noag-boot.sh would change:
    echo diff -c noag-boot.sh ${cfgfile}
    diff -c noag-boot.sh ${cfgfile}
  ) | ${PAGER-more}
fi

rm -rf ${tmpag}

# mkconfig.sh ends here
