#!/bin/sh
#
# A simple startup script for mysqld_multi by Tim Smith and Jani Tolonen.
# This script assumes that my.cnf file exists either in /etc/my.cnf or
# /root/.my.cnf and has groups [mysqld_multi] and [mysqldN]. See the
# mysqld_multi documentation for detailed instructions.
#
# This script can be used as /etc/init.d/mysql.server
#
# Comments to support chkconfig on RedHat Linux
# chkconfig: 2345 64 36
# description: A very fast and reliable SQL database engine.
#
# Version 1.0
#

basedir=/usr/local/mysql
bindir=/usr/local/mysql/bin

if test -x $bindir/mysqld_multi
then
  mysqld_multi="$bindir/mysqld_multi";
else
  echo "Can't execute $bindir/mysqld_multi from dir $basedir";
  exit;
fi

case "$1" in
    'start' )
        "$mysqld_multi" start $2
        ;;
    'stop' )
        "$mysqld_multi" stop $2
        ;;
    'report' )
        "$mysqld_multi" report $2
        ;;
    'restart' )
        "$mysqld_multi" stop $2
        "$mysqld_multi" start $2
        ;;
    *)
        echo "Usage: $0 {start|stop|report|restart}" >&2
        ;;
esac
