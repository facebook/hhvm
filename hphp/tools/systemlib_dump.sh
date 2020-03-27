#!/bin/bash

OBJDUMP=$(command -v objdump)
if [ ! -x "${OBJDUMP}" ]; then
  OBJDUMP=$(command -v gobjdump)
fi

function die {
  echo "$@" 2>&1
  exit 1
}

[[ -x "${OBJDUMP}" ]] || die "cannot locate objdump: ${OBJDUMP}"

[[ -n "$1" ]] || die "Usage: $0 <hhvm binary> [section name]"
[[ -r "$1" ]] || die "hhvm binary ${1} not readable"

SECTNAME=systemlib
if [ -n "$2" ]; then
  DIRNAME=$(dirname "$(readlink -e "${BASH_SOURCE[0]}")")
  SECTNAME=$("${DIRNAME}/section_name.sh" "$2")
fi

${OBJDUMP} -s -j "${SECTNAME}" "$1"
