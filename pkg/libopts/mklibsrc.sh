#! /bin/sh

set -e -x

top_builddir=`cd $top_builddir ; pwd`
top_srcdir=`cd $top_srcdir ; pwd`

tag=libopts-${AO_CURRENT}.${AO_REVISION}.${AO_AGE}

cd ${top_builddir}/pkg
[ ! -d ${tag} ] || rm -rf ${tag}
mkdir ${tag}
mkdir ${tag}/compat

cd ../autoopts
cp -f COPYING        \
      autoopts.c     \
      autoopts.h     \
      boolean.c      \
      enumeration.c  \
      genshell.c     \
      genshell.h     \
      numeric.c      \
      options.h      \
      pgusage.c      \
      restore.c      \
      save.c         \
      stack.c        \
      streqv.h       \
      streqvcmp.c    \
      usage.c        \
      version.c      \
  ../pkg/${tag}/.

cd ../compat
cp *.h pathfind.c ../pkg/${tag}/compat/.

cd ../pkg/${tag}

files=`ls -1 *.[ch] | \
	${top_builddir}/columns/columns -I4 --spread=1 --line='  \\' `

ag="${top_builddir}/agen5/autogen -L ${top_srcdir}/config --writable"
( echo "autogen definitions conftest.tpl;"
  echo "output-file = libopts.m4;"
  echo "group       = libopts;"
  cat ${top_srcdir}/config/libopts.def
) | $ag -DLIBOPTS=1

cat >> libopts.m4 <<-	EOMacro

	dnl @synopsis  AUTOOPTS_CHECK
	dnl
	dnl If autoopts-config works, add the linking information to
	dnl LIBS.  Otherwise, configure the LIBOPTS_DIR subdirectory
	dnl and run all the config tests that library needs.
	dnl
	AC_DEFUN([AUTOOPTS_CHECK],[
	  AC_MSG_CHECKING([whether autoopts-config can be found])
	  AC_CACHE_VAL([lo_cv_test_autoopts],[
	    lo_cv_test_autoopts=\`autoopts-config --libs\` 2> /dev/null
	    if [ \$? -ne 0 ]
	    then lo_cv_test_autoopts=no
	    else if [ -z "\$lo_cv_test_autoopts" ]
	         then lo_cv_test_autoopts=yes
	    fi ; fi
	  ]) # end of CACHE_VAL
	  AC_MSG_RESULT([\${lo_cv_test_autoopts}])
	
	  if test "X\${lo_cv_test_autoopts}" != Xno
	  then
	    LIBS="\${LIBS} \${lo_cv_test_autoopts}"
	  else
	    INVOKE_LIBOPTS_MACROS
	    LIBOPTS_DIR=${tag}
	    AC_SUBST( LIBOPTS_DIR )
	  fi
	]) # end of AC_DEFUN of AUTOOPTS_CHECK
	EOMacro

[ -f Makefile.am ] && rm -f Makefile.am

cat > Makefile.am <<-	EOMakefile

	MAINTAINERCLEANFILES = Makefile.in
	INCLUDES = @INCLIST@
	SRC      = \\
	$files
	lib_LTLIBRARIES = libopts.la
	libopts_la_SOURCES = \$(SRC)
	EOMakefile

sed s,'\${tag}',"${tag}",g ../libopts/README > README
cp ../libopts/COPYING* .

cd ..
tar cvf - ${tag} | gzip --best > ${top_builddir}/${tag}.tar.gz
rm -rf ${tag}
