#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

# Utility method for generating the header and source code for a generic perfect
# hash table data structure by leveraging gperf, input template header and gperf
# source files, and input word list files that respresent the enum fields.
function generate_perfect_hash_table {

  # $1 is a space separated list of files that store words that will correspond
  #    to output enum fields.
  # $2 is the path to the input template header file.
  # $3 is the path to the 'awk' script that will output format the template
  #    header file.
  # $4 is the desired output path of the generated header file.
  # $5 is the path to the input template gperf file.
  # $6 is the path to the 'awk' script that will format the template gperf file
  #    prior to it being passed to the gperf binary.
  # $7 is the desired output path of the generated source file.
  # $8 is the optional path to the gperf binary to use.

  # The inputs to 'cat' below are not quoted as they are space separated lists
  # of strings (files); 'cat' will concatenate them (if applicable) and pipe the
  # combined output onwards to 'awk'.

  # Generate output header file.
  LC_ALL=C sort -u ${1?} | awk "${3?}" - "${2?}" > "${4?}"

  # Generate output source file.
  # We also sub out the default 'FALLTHROUGH' comment statements as modern
  # compilers will raise warnings unless the fallthrough is explicit.
  LC_ALL=C sort -u ${1?} | awk "${6?}" - "${5?}" | \
  ${8:-gperf} -m5 -D --output-file="${7?}"
  perl -p -i -e "s/\/\*FALLTHROUGH\*\//[[fallthrough]];/g" "${7?}"

  # Here we delete one of the comment lines gperf adds to the top of the file.
  # i.e. /* Command-line: .../gperf -m5 --output-file=...  */
  # The reason this line is removed is the '...'s are actually expanded to
  # absolute paths on the build machine which prevents buck's cache from being
  # used effectively.
  # Prefer perl to sed so that it works on both Mac and Linux.
  perl -n -i -e "print unless m /\\/* Command-line: /" "${7?}"
}
