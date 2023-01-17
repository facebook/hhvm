#!/bin/bash
DIRECTORY=.
out=${1:-dune.inc}
echo "; GENERATED RULES" > "$out"


for filename in "$DIRECTORY"/test_*_embed_error.ml; do
    file=$(basename -- "$filename");
    pfx="${file%.*}";
    echo "
(rule
 (targets $pfx.actual.ml)
 (deps
  (:pp pp.exe)
  (:input $file))
 (action
   (progn
     (with-stdout-to %{targets} (run ./%{pp} --impl %{input} -o %{targets}))
     (bash \"arc f %{targets} > /dev/null 2>&1\")
   )
 )
)
(rule
 (alias runtest)
 (action
  (diff $pfx.expected.ml $pfx.actual.ml)))" >> "$out";
done
