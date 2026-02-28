#!/bin/sh
# Copyright (c) 2000, 2016, Oracle and/or its affiliates. All rights reserved.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

# This script reports various configuration settings that may be needed
# when using the MySQL client library.

which ()
{
  IFS="${IFS=   }"; save_ifs="$IFS"; IFS=':'
  for file
  do
    for dir in $PATH
    do
      if test -f $dir/$file
      then
        echo "$dir/$file"
        continue 2
      fi
    done
    echo "which: no $file in ($PATH)"
    exit 1
  done
  IFS="$save_ifs"
}

#
# If we can find the given directory relatively to where mysql_config is
# we should use this instead of the incompiled one.
# This is to ensure that this script also works with the binary MySQL
# version

fix_path ()
{
  var=$1
  shift
  for filename
  do
    path=$basedir/$filename
    if [ -d "$path" ] ;
    then
      eval "$var"=$path
      return
    fi
  done
}

get_full_path ()
{
  file=$1

  # if the file is a symlink, try to resolve it
  if [ -h $file ];
  then
    file=`ls -l $file | awk '{ print $NF }'`
  fi

  case $file in
    /*) echo "$file";;
    */*) tmp=`pwd`/$file; echo $tmp | sed -e 's;/\./;/;' ;;
    *) which $file ;;
  esac
}

me=`get_full_path $0`

# Script might have been renamed but assume mysql_<something>config<something>
basedir=`echo $me | sed -e 's;/bin/mysql_.*config.*;;'`

ldata='@localstatedir@'
execdir='@libexecdir@'
bindir='@bindir@'

# If installed, search for the compiled in directory first (might be "lib64")
pkglibdir='@pkglibdir@'
pkglibdir_rel=`echo $pkglibdir | sed -e "s;^$basedir/;;"`
fix_path pkglibdir $pkglibdir_rel @libsubdir@/mysql @libsubdir@

plugindir='@pkgplugindir@'
plugindir_rel=`echo $plugindir | sed -e "s;^$basedir/;;"`
fix_path plugindir $plugindir_rel @libsubdir@/mysql/plugin @libsubdir@/plugin

pkgincludedir='@pkgincludedir@'
if [ -f "$basedir/include/mysql/mysql.h" ]; then
  pkgincludedir="$basedir/include/mysql"
elif [ -f "$basedir/include/mysql.h" ]; then
  pkgincludedir="$basedir/include"
fi

version='@VERSION@'
socket='@MYSQL_UNIX_ADDR@'
ldflags='@LDFLAGS@'

if [ @MYSQL_TCP_PORT_DEFAULT@ -eq 0 ]; then
  port=0
else
  port=@MYSQL_TCP_PORT@
fi

# Create options 
libs="@QUOTED_CMAKE_CXX_LINK_FLAGS@-L$pkglibdir@RPATH_OPTION@"
libs="$libs -l@LIBMYSQL_OS_OUTPUT_NAME@ @CONFIG_CLIENT_LIBS@"

cflags="-I$pkgincludedir @CFLAGS@"
cxxflags="-I$pkgincludedir @CXXFLAGS@"
include="-I$pkgincludedir"


usage () {
        cat <<EOF
Usage: $0 [OPTIONS]
Compiler: @COMPILER_ID_AND_VERSION@
Options:
        --cflags         [$cflags]
        --cxxflags       [$cxxflags]
        --include        [$include]
        --libs           [$libs]
        --libs_r         [$libs]
        --plugindir      [$plugindir]
        --socket         [$socket]
        --port           [$port]
        --version        [$version]
        --variable=VAR   VAR is one of:
                pkgincludedir [$pkgincludedir]
                pkglibdir     [$pkglibdir]
                plugindir     [$plugindir]
EOF
        exit 1
}

if test $# -le 0; then usage; fi

while test $# -gt 0; do
        case $1 in
        --cflags)  echo "$cflags" ;;
        --cxxflags)echo "$cxxflags";;
        --include) echo "$include" ;;
        --libs)    echo "$libs" ;;
        --libs_r)  echo "$libs" ;;
        --plugindir) echo "$plugindir" ;;
        --socket)  echo "$socket" ;;
        --port)    echo "$port" ;;
        --version) echo "$version" ;;
        --variable=*)
          var=`echo "$1" | sed 's,^[^=]*=,,'`
          case "$var" in
            pkgincludedir) echo "$pkgincludedir" ;;
            pkglibdir) echo "$pkglibdir" ;;
            plugindir) echo "$plugindir" ;;
            *) usage ;;
          esac
          ;;
        *)         usage ;;
        esac

        shift
done

#echo "ldata: '"$ldata"'"
#echo "execdir: '"$execdir"'"
#echo "bindir: '"$bindir"'"
#echo "pkglibdir: '"$pkglibdir"'"
#echo "pkgincludedir: '"$pkgincludedir"'"
#echo "version: '"$version"'"
#echo "socket: '"$socket"'"
#echo "port: '"$port"'"
#echo "ldflags: '"$ldflags"'"
#echo "client_libs: '"$client_libs"'"

exit 0
