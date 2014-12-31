#!/bin/sh
# This is a terrible hack. This is necessary because there is no way to change
# the working directory in hhvm before we exec (chdir only changes a var in the
# execution context). We need to change the working directory because php5
# xdebug only accepts filenames relative the working directory and so line
# breakpoint tests would fail if not run from the root test directory.
# We need to manually fork and exec (as opposed to using proc_open) because we
# have to close the listening socket before we exec.

# arg 1 is the working directory to cd to
# arg 2 is the hhvm binary
# arg 3 is "-c"
# arg 4 is the ini file
# arg 5 is the test to run
cd $1
exec $2 $3 $4 $5
