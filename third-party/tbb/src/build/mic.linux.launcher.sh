#!/bin/bash
#
# Copyright (c) 2005-2018 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#
#
#

# Usage:
# mic.linux.launcher.sh [-v] [-q] [-s] [-r <repeats>] [-u] [-l <library>] <executable> <arg1> <arg2> <argN>
#         where: -v enables verbose output
#         where: -q enables quiet mode
#         where: -s runs the test in stress mode (until non-zero exit code or ctrl-c pressed)
#         where: -r <repeats> specifies number of times to repeat execution
#         where: -u limits stack size
#         where: -l <library> specifies the library name to be assigned to LD_PRELOAD
#
# Libs and executable necessary for testing should be present in the current directory before running.
# Note: Do not remove the redirections to '/dev/null' in the script, otherwise the nightly test system will fail.
#
trap 'echo Error at line $LINENO while executing "$BASH_COMMAND"' ERR #
trap 'echo -e "\n*** Interrupted ***" && exit 1' SIGINT SIGQUIT #
# Process the optional arguments if present
while getopts  "qvsr:ul:" flag #
do case $flag in #
    s )  # Stress testing mode
         echo Doing stress testing. Press Ctrl-C to terminate
         run_env='stressed() { while $*; do :; done; };' #
         run_prefix="stressed $run_prefix" ;; #
    r )  # Repeats test n times
         run_env="repeated() { for i in \$(seq 1 $OPTARG); do echo \$i of $OPTARG:; \$*; done; };" #
         run_prefix="repeated $run_prefix" ;; #
    l )  # Additional library
         ldd_list+="$OPTARG " #
         run_prefix+=" LD_PRELOAD=$OPTARG" ;; #
    u )  # Set stack limit
         run_prefix="ulimit -s 10240; $run_prefix" ;; # 
    q )  # Quiet mode, removes 'done' but prepends any other output by test name
         SUPPRESS='>/dev/null' #
         verbose=1 ;; # TODO: implement a better quiet mode
    v )  # Verbose mode
         verbose=1 ;; #
esac done #
shift `expr $OPTIND - 1` #
[ $verbose ] || SUPPRESS='>/dev/null' #
#
# Collect the executable name
fexename="$1" #
exename=`basename $1` #
shift #
#
: ${MICDEV:=mic0} #
RSH="sudo ssh $MICDEV" #
RCP="sudo scp" #
currentdir=$PWD #
#
# Prepare the target directory on the device
targetdir="`$RSH mktemp -d /tmp/tbbtestXXXXXX 2>/dev/null`" #
# Prepare the temporary directory on the host
hostdir="`mktemp -d /tmp/tbbtestXXXXXX 2>/dev/null`" #
#
function copy_files { #
    [ $verbose ] && echo Going to copy $* #
    eval "cp $* $hostdir/ $SUPPRESS 2>/dev/null || exit \$?" #
    eval "$RCP $hostdir/* $MICDEV:$targetdir/ $SUPPRESS 2>/dev/null || exit \$?" #
    eval "rm $hostdir/* $SUPPRESS 2>/dev/null || exit \$?" #
} # copy files
#
function clean_all() { #
    eval "$RSH rm -fr $targetdir $SUPPRESS" ||: #
    eval "rm -fr $hostdir $SUPPRESS" ||: #
} # clean all temporary files
#
function kill_interrupt() { #
    echo -e "\n*** Killing remote $exename ***" && $RSH "killall $exename" #
    clean_all #
} # kill target process
#
trap 'clean_all' SIGINT SIGQUIT # trap keyboard interrupt (control-c)
#
# Transfer the test executable file and its auxiliary libraries (named as {test}_dll.so) to the target device.
copy_files $fexename `ls ${exename%\.*}*.so 2>/dev/null ||:` #
#
# Collect all dependencies of the test and its auxiliary libraries to transfer them to the target device.
ldd_list+="libtbbmalloc*.so* libirml*.so* `$RSH ldd $targetdir/\* | grep = | cut -d= -f1 2>/dev/null`" #
fnamelist="" #
#
# Find the libraries and add them to the list.
# For example, go through MIC_LD_LIBRARY_PATH and add TBB libraries from the first
# directory that contains tbb files
mic_dir_list=`echo .:$MIC_LD_LIBRARY_PATH | tr : " "` #
[ $verbose ] && echo Searching libraries in $mic_dir_list
for name in $ldd_list; do # adds the first matched name in specified dirs
    found="`find -L $mic_dir_list -name $name -a -readable -print -quit 2>/dev/null` "||: #
    [ $verbose ] && echo File $name: $found
    fnamelist+=$found
done #
#
# Remove extra spaces.
fnamelist=`echo $fnamelist` #
# Transfer collected executable and library files to the target device.
[ -n "$fnamelist" ] && copy_files $fnamelist
#
# Transfer input files used by example codes by scanning the executable argument list.
argfiles= #
args= #
for arg in "$@"; do #
  if [ -r $arg ]; then #
    argfiles+="$arg " #
    args+="$(basename $arg) " #
  else #
    args+="$arg " #
  fi #
done #
[ -n "$argfiles" ] && copy_files $argfiles #
#
# Get the list of transferred files
testfiles="`$RSH find $targetdir/ -type f | tr '\n' ' ' 2>/dev/null`" #
#
[ $verbose ] && echo Running $run_prefix ./$exename $args #
# Run the test on the target device
trap 'kill_interrupt' SIGINT SIGQUIT # trap keyboard interrupt (control-c)
trap - ERR #
run_env+="cd $targetdir; export LD_LIBRARY_PATH=.:\$LD_LIBRARY_PATH;" #
$RSH "$run_env $run_prefix ./$exename $args" #
#
# Delete the test files and get the list of output files
outfiles=`$RSH rm $testfiles 2>/dev/null; find $targetdir/ -type f 2>/dev/null` ||: #
if [ -n "$outfiles" ]; then #
    for outfile in $outfiles; do #
        filename=$(basename $outfile) #
        subdir=$(dirname $outfile) #
        subdir="${subdir#$targetdir}" #
        [ -n $subdir ] subdir=$subdir/ #
        # Create directories on host
        [ ! -d "$hostdir/$subdir" ] && mkdir -p "$hostdir/$subdir" #
        [ ! -d "$currentdir/$subdir" ] && mkdir -p "$currentdir/$subdir" #
        # Copy the output file to the temporary directory on host
        eval "$RCP -r '$MICDEV:${outfile#}' '$hostdir/$subdir$filename' $SUPPRESS 2>&1 || exit \$?" #
        # Copy the output file from the temporary directory to the current directory
        eval "cp '$hostdir/$subdir$filename' '$currentdir/$subdir$filename' $SUPPRESS 2>&1 || exit \$?" #
    done #
fi #
#
# Clean up temporary directories
clean_all
#
# Return the exit code of the test.
exit $? #
