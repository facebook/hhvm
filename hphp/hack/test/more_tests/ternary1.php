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

function f1(): ?string {
  if (true) {
    return 'x';
  } else {
    return null;
  }
}

function f2(): ?int {
  if (true) {
    return 42;
  } else {
    return null;
  }
}

function t(): ?string {
  return true ? f1() : f2();
}
