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

enum NumbahsEnum : int as int {
  value1 = 1;
  value2 = 2;
  lifeValue = 42;
}

function bar(classname<NumbahsEnum> $test): int {
  return NumbahsEnum::lifeValue;
}

function foo(): int {
  return bar(NumbahsEnum::class);
}
