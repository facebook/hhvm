#!/bin/sh
set -e

cd `dirname $0`
./cleanup.sh

if test -f m4/ax_lib_event.m4 ; then : ; else
  if test -d m4 ; then : ; else
    mkdir m4
  fi
  echo >&2 "Please copy ax_lib_event.m4 from Thrift (under aclocal)"
  echo >&2 "to the 'm4' directory.  Or run"
  echo >&2 "  wget -O m4/ax_lib_event.m4 'http://gitweb.thrift-rpc.org/?p=thrift.git;a=blob_plain;f=aclocal/ax_lib_event.m4;hb=HEAD'"
  echo >&2 "Then re-run bootstrap.sh"
  exit 1
fi

autoscan
libtoolize --automake
autoheader
aclocal -I m4
autoconf
automake --add-missing
