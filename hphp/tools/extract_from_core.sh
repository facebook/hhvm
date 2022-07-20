#!/bin/bash -e

if [[ $# -ne 4 ]]; then
   echo "Usage: $0 hhvm_binary hhvm_core type outfile"
   echo "  hhvm_binary: the binary corresponding to the core file"
   echo "  hhvm_core: the core file we want to extract data from"
   echo "  type: the type of data we want to extract 'jitprof' or 'stacktrace'"
   echo "  outfile: the file to store the extracted data in"
   exit 1
fi

binary="$1"
core="$2"
datatype="$3"
outfile="$4"

binary="$(realpath "$binary")"
outdir="$(dirname "$outfile")"
outbasefile="$(basename "$outfile")"

# GDB can't dump outside the current working directory without coring
pushd "$outdir"
gdb "$binary" "$core" -batch \
  -ex "dump binary memory $outbasefile 'HPHP::s_${datatype}_start' 'HPHP::s_${datatype}_end'" \
  -ex "quit"
popd
