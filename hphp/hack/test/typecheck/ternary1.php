<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function f1(): ?string {
  if (cond()) {
    return 'x';
  } else {
    return null;
  }
}

function f2(): ?int {
  if (cond()) {
    return 42;
  } else {
    return null;
  }
}

function t(): ?string {
  return cond() ? f1() : f2();
}

function cond(): bool {
  return true;
}
