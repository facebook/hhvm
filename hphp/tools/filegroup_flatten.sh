#!/bin/bash -e
OUT=$1

for item in "${@:2}"
do
  if [[ -d $item ]]; then
    if [ -n "$(ls -A "$item")" ]; then
        cp "$item"/* "$OUT"
    fi
  elif [[ -f $item ]]; then
    cp "$item" "$OUT"
  fi
done
