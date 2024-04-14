#!/bin/bash -e

if [[ $# -lt 4 ]]; then
   echo "Usage: $0 hhvm_binary hhvm_core type outfile [--hhvm_debuginfo hhvm.debuginfo]"
   echo "  hhvm_binary: the binary corresponding to the core file"
   echo "  hhvm_core: the core file we want to extract data from"
   echo "  type: the type of data we want to extract 'jitprof' or 'stacktrace'"
   echo "  outfile: the file to store the extracted data in"
   echo "  --hhvm_debuginfo hhvm.debuginfo: debuginfo file"
   exit 1
fi

binary="$1"
core="$2"
datatype="$3"
outfile="$4"

binary="$(realpath "$binary")"
outdir="$(dirname "$outfile")"
outbasefile="$(basename "$outfile")"

if [ -n "$5" ]; then
  debuginfo=$5
else
  debuginfo="$(dirname "$binary")/hhvm.debuginfo"
fi

if [ "$USE_GDB" == "1" ]; then
  # GDB can't dump outside the current working directory without coring
  pushd "$outdir"
  gdb "$binary" "$core" -batch \
    -ex "dump binary memory $outbasefile 'HPHP::s_${datatype}_start' 'HPHP::s_${datatype}_end'" \
    -ex "quit"
  popd
else
  temp_file=$(mktemp)
  # Using -f c-string doesn't work because it stops at the first null terminator, despite the end address given
  lldb "$binary" -c "$core" --batch \
    -o "target symbols add $debuginfo" \
    -o "memory read --force -f character -o $temp_file s_${datatype}_start s_${datatype}_end"
  sed -i -E 's/^0x[0-9a-f]+: //' "$temp_file"
  tr -d '\n' <"$temp_file" >"$outbasefile"
  sed -i 's/\\n/\n/g' "$outbasefile"
fi
