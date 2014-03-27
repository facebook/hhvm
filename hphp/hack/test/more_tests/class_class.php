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

class C {}
trait Tr {}
interface I {}

function foo(): string {
  print_string(Tr::class);
  print_string(C::class);
  print_string(I::class);
  return C::class;
}

function print_string(string $s): void {
  echo $s, "\n";
}
