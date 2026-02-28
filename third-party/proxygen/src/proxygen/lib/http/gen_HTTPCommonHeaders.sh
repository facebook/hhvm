#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

set -e

# Setup and validate arguments

if [ "x$1" != "x" ]; then
  ENUM_FILE_LIST="$1"
fi
if [ "x$2" != "x" ];then
  FBCODE_DIR="$2"
fi
if [ "x$3" != "x" ]; then
  OUTPUT_DIR="$3"
fi
if [ "x$4" != "x" ]; then
  GPERF="$4"
fi

# The `awk` scripts aren't nearly as hairy as it seems. The only real trick is
# the first line. We're processing two files -- the result of a `cat` pipeline
# above, plus a template. The "NR == FNR" compares the current file's
# line number with the total number of lines we've processed -- i.e., that test
# means "am I in the first file?" So we suck those lines aside. Then we process
# the second file, replacing "%%%%%" with some munging of the lines we sucked
# aside from the `cat` pipeline. They are written in awk since it isn't really
# worth the build system BS of calling out to Python (which is unfortunately
# particularly annoying in Facebook's internal build system) and more portable
# (and less hairy honestly) than the massive `sed` pipeline which used to be
# here. And it really isn't that bad, this is the sort of thing `awk` was
# designed for.

AWK_HEADER_SCRIPT='
  NR == FNR {
    n[FNR] = $1;
    max = FNR;
    next
  }
  $1 == "%%%%%" {
    for (i in n) {
      h = n[i];
      gsub("-", "_", h);
      gsub(":", "COLON_", h);
      print "  HTTP_HEADER_" toupper(h) " = " i+1 ","
    };
    next
  }
  $1 == "$$$$$" {
    print "  constexpr static uint64_t num_codes = " max+2 ";";
    next
  }
  {
    gsub("%%name%%", "HTTPCommonHeaders");
    gsub("%%name_enum%%", "HTTPHeaderCode");
    gsub("%%enum_prefix%%", "HTTP_HEADER");
    gsub("%%table_type_name%%", "HTTPCommonHeaderTableType");
    print
  }
'

AWK_SOURCE_SCRIPT='
  NR == FNR {
    n[FNR] = $1;
    next
  }
  $1 == "%%%%%" {
    print "%%";
    for (i in n) {
      h = n[i];
      gsub("-", "_", h);
      gsub(":", "COLON_", h);
      print n[i] ", HTTP_HEADER_" toupper(h)
    };
    print "%%";
    next
  }
  {
    gsub("%%name%%", "HTTPCommonHeaders");
    gsub("%%name_internal%%", "HTTPCommonHeadersInternal");
    gsub("%%name_container%%", "HTTPCommonHeaderName");
    gsub("%%name_enum%%", "HTTPHeaderCode");
    gsub("%%header%%", "proxygen/lib/http/HTTPCommonHeaders.h");
    gsub("%%enum_other%%", "HTTP_HEADER_OTHER");
    gsub("%%table_type_name%%", "HTTPCommonHeaderTableType");
    print
  }
'

# Load up the utility method for generating our desired perfect hash table
# shellcheck source=/dev/null
. "${FBCODE_DIR?}/proxygen/lib/utils/gen_perfect_hash_table.sh"

generate_perfect_hash_table \
  "${ENUM_FILE_LIST}" \
  "${FBCODE_DIR}/proxygen/lib/utils/perfect_hash_table_template.h" \
  "${AWK_HEADER_SCRIPT}" \
  "${OUTPUT_DIR}/HTTPCommonHeaders.h" \
  "${FBCODE_DIR}/proxygen/lib/utils/perfect_hash_table_template.cpp.gperf" \
  "${AWK_SOURCE_SCRIPT}" \
  "${OUTPUT_DIR?}/HTTPCommonHeaders.cpp" \
  "${GPERF}"
