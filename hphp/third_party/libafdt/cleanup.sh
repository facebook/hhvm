#!/bin/sh

cd `dirname $0`

make -k distclean >/dev/null 2>&1

xargs rm -rf <<EOL
Makefile.in
aclocal.m4
autom4te.cache
autoscan.log
config.guess
config.h.in
config.h.in~
config.sub
configure
configure.scan
depcomp
ltmain.sh
missing
m4/libtool.m4
m4/lt~obsolete.m4
m4/ltoptions.m4
m4/ltsugar.m4
m4/ltversion.m4
EOL
# Separate this one to avoid a silly autoscan warning
rm -f "i"nstall-sh

true
