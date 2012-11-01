dnl 
dnl AUTHOR: Francesco Chemolli
dnl
dnl SQUID Web Proxy Cache          http://www.squid-cache.org/
dnl ----------------------------------------------------------
dnl Squid is the result of efforts by numerous individuals from
dnl the Internet community; see the CONTRIBUTORS file for full
dnl details.   Many organizations have provided support for Squid's
dnl development; see the SPONSORS file for full details.  Squid is
dnl Copyrighted (C) 2001 by the Regents of the University of
dnl California; see the COPYRIGHT file for full details.  Squid
dnl incorporates software developed and/or copyrighted by other
dnl sources; see the CREDITS file for full details.
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.

dnl save main environment variables to variables to the namespace defined by the
dnl first argument (prefix)
dnl e.g. SQUID_SAVEFLAGS([foo]) will save CFLAGS to foo_CFLAGS etc.
dnl Saved variables are:
dnl CFLAGS, CXXFLAGS, LDFLAGS, LIBS plus any variables specified as
dnl second argument
AC_DEFUN([SQUID_STATE_SAVE],[
# save state, key is $1
$1_CFLAGS="${CFLAGS}"
$1_CXXFLAGS="${CXXFLAGS}"
$1_LDFLAGS="${LDFLAGS}"
$1_LIBS="${LIBS}"
$1_CC="${CC}"
$1_CXX="${CXX}"
$1_squid_saved_vars="$2"
for squid_util_var_tosave in $$1_squid_saved_vars
do
    squid_util_var_tosave2="$1_${squid_util_var_tosave}"
    eval "${squid_util_var_tosave2}=\"${squid_util_var_tosave}\""
done
])

dnl commit the state changes: deleting the temporary state defined in SQUID_STATE_SAVE
dnl with the same prefix. It's not necessary to specify the extra variables passed
dnl to SQUID_STATE_SAVE again, they will be automatically reclaimed.
AC_DEFUN([SQUID_STATE_COMMIT],[
# commit state, key is $1
unset $1_CFLAGS
unset $1_CXXFLAGS
unset $1_LDFLAGS
unset $1_LIBS
unset $1_CC
unset $1_CXX
for squid_util_var_tosave in $$1_squid_saved_vars
do
    unset ${squid_util_var_tosave}
done
])

dnl rollback state to the call of SQUID_STATE_SAVE with the same namespace argument.
dnl all temporary state will be cleared, including the custom variables specified
dnl at call time. It's not necessary to explicitly name them, they will be automatically
dnl cleared.
AC_DEFUN([SQUID_STATE_ROLLBACK],[
# rollback state, key is $1
CFLAGS="${$1_CFLAGS}"
CXXFLAGS="${$1_CXXFLAGS}"
LDFLAGS="${$1_LDFLAGS}"
LIBS="${$1_LIBS}"
CC="${$1_CC}"
CXX="${$1_CXX}"
for squid_util_var_tosave in $$1_squid_saved_vars
do
    squid_util_var_tosave2="\$$1_${squid_util_var_tosave}"
    eval "$squid_util_var_tosave=\"${squid_util_var_tosave2}\""
done
SQUID_STATE_COMMIT($1)
])


dnl look for modules in the base-directory supplied as argument.
dnl fill-in the variable pointed-to by the second argument with the
dnl space-separated list of modules 
AC_DEFUN([SQUID_LOOK_FOR_MODULES],[
$2=""
for dir in $1/*; do
  module="`basename $dir`"
  if test -d "$dir" && test "$module" != CVS; then
      $2="$$2 $module"
  fi
done
])

dnl remove duplicates out of a list.
dnl dnl argument is the name of a variable to be checked and cleaned up
AC_DEFUN([SQUID_CLEANUP_MODULES_LIST],[
squid_cleanup_tmp_outlist=""
for squid_cleanup_tmp in $$1
do
  squid_cleanup_tmp_dupe=0
  for squid_cleanup_tmp2 in $squid_cleanup_tmp_outlist
  do
    if test "$squid_cleanup_tmp" = "$squid_cleanup_tmp2"; then
      squid_cleanup_tmp_dupe=1
      break
    fi
  done
  if test $squid_cleanup_tmp_dupe -eq 0; then
    squid_cleanup_tmp_outlist="${squid_cleanup_tmp_outlist} $squid_cleanup_tmp"
  fi
done
$1=$squid_cleanup_tmp_outlist
unset squid_cleanup_tmp_outlist
unset squid_cleanup_tmp_dupe
unset squid_cleanup_tmp2
unset squid_cleanup_tmp
])

dnl check that all the modules supplied as a whitespace-separated list (second
dnl argument) exist as members of the basedir passed as first argument
dnl call AC_MESG_ERROR if any module does not exist. Also sets individual variables
dnl named $2_modulename to value "yes"
dnl e.g. SQUID_CHECK_EXISTING_MODULES([$srcdir/src/fs],[foo_module_candidates])
dnl where $foo_module_candidates is "foo bar gazonk"
dnl checks whether $srcdir/src/fs/{foo,bar,gazonk} exist and are all dirs
dnl AND sets $foo_module_candidates_foo, $foo_module_candidates_bar 
dnl and $foo_module_candidates_gazonk to "yes"
AC_DEFUN([SQUID_CHECK_EXISTING_MODULES],[
  for squid_module_check_exist_tmp in $$2
  do
    if test -d $1/$squid_module_check_exist_tmp
    then
      eval "$2_$squid_module_check_exist_tmp='yes'"
      #echo "defining $2_$squid_module_check_exist_tmp"
    else
      AC_MSG_ERROR([$squid_module_check_exist_tmp not found in $1])
    fi
  done
])

dnl lowercases the contents of the variable whose name is passed by argument
AC_DEFUN([SQUID_TOLOWER_VAR_CONTENTS],[
  $1=`echo $$1|tr ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz`
])

dnl uppercases the contents of the variable whose name is passed by argument
AC_DEFUN([SQUID_TOUPPER_VAR_CONTENTS],[
  $1=`echo $$1|tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ`
])

dnl like AC_DEFINE, but it defines the value to 0 or 1 using well-known textual
dnl conventions:
dnl 1: "yes", "true", 1
dnl 0: "no" , "false", 0, ""
dnl aborts with an error for unknown values
AC_DEFUN([SQUID_DEFINE_BOOL],[
squid_tmp_define=""
case "$2" in 
  yes|true|1) squid_tmp_define="1" ;;
  no|false|0|"") squid_tmp_define="0" ;;
  *) AC_MSG_ERROR([SQUID_DEFINE[]_BOOL: unrecognized value for $1: '$2']) ;;
esac
ifelse([$#],3, 
  [AC_DEFINE_UNQUOTED([$1], [$squid_tmp_define],[$3])],
  [AC_DEFINE_UNQUOTED([$1], [$squid_tmp_define])]
)
unset squid_tmp_define
])

dnl aborts with an error specified as the second argument if the first argument doesn't
dnl contain either "yes" or "no"
AC_DEFUN([SQUID_YESNO],[
if test "$1" != "yes" -a "$1" != "no" ; then
  AC_MSG_ERROR([$2])
fi
])

AC_DEFUN([SQUID_EMBED_BUILD_INFO],[
  AC_ARG_ENABLE([build-info],
    AS_HELP_STRING([--enable-build-info="build info string"],
      [Add an additional string in the output of "squid -v".
       Default is not to add anything. If the string is not specified,
       tries to determine nick and revision number of the current 
       bazaar branch]),[
  case "$enableval" in
    no) ${TRUE}
        ;;
    yes)
      if test -d "${srcdir}/.bzr"; then
        AC_PATH_PROG(BZR,bzr,$FALSE)
        squid_bzr_branch_nick=`cd ${srcdir} && ${BZR} nick 2>/dev/null`
        if test $? -eq 0 -a "x$squid_bzr_branch_nick" != "x"; then
          squid_bzr_branch_revno=`cd ${srcdir} && ${BZR} revno 2>/dev/null | sed 's/\"//g'`
        fi
        if test $? -eq 0 -a "x$squid_bzr_branch_revno" != "x"; then
          sh -c "cd ${srcdir} && ${BZR} diff 2>&1 >/dev/null"
          if test $? -eq 1; then
              squid_bzr_branch_revno="$squid_bzr_branch_revno+changes"
          fi
        fi
        if test "x$squid_bzr_branch_revno" != "x"; then
          squid_build_info="Built branch: ${squid_bzr_branch_nick}-r${squid_bzr_branch_revno}"
        fi
      fi
      ;;
    *)
      squid_build_info=$enableval
      ;;
  esac
  ])
  if test "x${squid_build_info:=no}" != "xno"; then
    AC_DEFINE_UNQUOTED([SQUID_BUILD_INFO],["$squid_build_info"],
       [Squid extended build info field for "squid -v" output])
  fi
])

dnl like AC_SEARCH_LIBS, with an extra argument which is
dnl a prefix to the test program
AC_DEFUN([SQUID_SEARCH_LIBS],
[AS_VAR_PUSHDEF([ac_Search], [ac_cv_search_$1])dnl
AC_CACHE_CHECK([for library containing $1], [ac_Search],
[ac_func_search_save_LIBS=$LIBS
AC_LANG_CONFTEST([AC_LANG_PROGRAM([$6], [$1()])])
for ac_lib in '' $2; do
  if test -z "$ac_lib"; then
    ac_res="none required"
  else
    ac_res=-l$ac_lib
    LIBS="-l$ac_lib $5 $ac_func_search_save_LIBS"
  fi
  AC_LINK_IFELSE([], [AS_VAR_SET([ac_Search], [$ac_res])])
  AS_VAR_SET_IF([ac_Search], [break])
done
AS_VAR_SET_IF([ac_Search], , [AS_VAR_SET([ac_Search], [no])])
rm conftest.$ac_ext
LIBS=$ac_func_search_save_LIBS])
ac_res=AS_VAR_GET([ac_Search])
AS_IF([test "$ac_res" != no],
  [test "$ac_res" = "none required" || LIBS="$ac_res $LIBS"
  $3],
      [$4])
AS_VAR_POPDEF([ac_Search])dnl
])
