<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

namespace NS;

function returns_int(mixed $in): ?int {
  if ($in is int) {
    return $in;
  }
  return null;
}

function returns_vec(mixed $in): ?vec<mixed> {
  if ($in is vec<_>) {
    return $in;
  }
  return null;
}
