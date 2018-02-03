<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

namespace NS;

function returns_int(mixed $in): ?int {
  if (is_int($in)) {
    return $in;
  }
  return null;
}

/* HH_IGNORE_ERROR[4045] array without generics */
function returns_array(mixed $in): ?array {
  if (is_array($in)) {
    return $in;
  }
  return null;
}

function returns_vec(mixed $in): ?vec<mixed> {
  if (is_vec($in)) {
    return $in;
  }
  return null;
}
