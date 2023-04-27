# Copyright (C) 2016 and later: Unicode, Inc. and others.
# License & terms of use: http://www.unicode.org/copyright.html
# Copyright (C) 2006-2011, International Business Machines Corporation
# and others.  All Rights Reserved.
#
# Use "test -x" instead of "test -f" most of the time.
# due to how executables are created in a different file system.
s/as_executable_p="test -f"/as_executable_p="test -x"/g
s/test -f "$ac_file"/test -x "$ac_file"/g
s/test -f $ac_dir\/install-sh/test -x $ac_dir\/install-sh/g
s/test -f $ac_dir\/install.sh/test -x $ac_dir\/install.sh/g
s/test -f $ac_dir\/shtool/test -x $ac_dir\/shtool/g
# Use the more efficient del instead of rm command.
s/rm[ ]*-r[ ]*-f/del -f/g
s/rm[ ]*-f[ ]*-r/del -f/g
s/rm[ ]*-rf/del -f/g
s/rm[ ]*-fr/del -f/g
s/rm[ ]*-f/del -f/g
##don't clean up some awks for debugging
#s/[ ]*del -f [^ ]*.awk/#&/
# Borne shell isn't always available on i5/OS
s/\/bin\/sh/\/usr\/bin\/qsh/g
# no diff in qsh the equivalent is cmp
s/ diff / cmp -s /g
## srl
# trouble w/ redirects.
s% >&$3%%g
s% >&$4% >$4%g
s%^ac_cr=%# AWK reads ASCII, not EBCDIC\
touch -C 819 $tmp/defines.awk $tmp/subs.awk $tmp/subs1.awk conf$$subs.awk\
\
&%
##OBSOLETE
#(REPLACED BY CPP in runConfigureICU) Use -c qpponly instead of -E to enable the preprocessor on the compiler
#s/\$CC -E/\$CC -c -qpponly/g
