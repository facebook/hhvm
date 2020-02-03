<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function f(): array {
  return varray[1];
}

function g(): array<int> {
  return varray[1];
}

function h(): array<string, int> {
  return darray["one" => 1];
}
