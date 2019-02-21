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
  return [1];
}

function g(): array<int> {
  return [1];
}

function h(): array<string, int> {
  return ["one" => 1];
}
