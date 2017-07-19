<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

interface MyPlainSprintf {
  public function format_s(mixed $s): string;
}

function my_sprintf(FormatString<MyPlainSprintf> $f, ...): string {
  return 'hi';
}

function f() {
  my_sprintf('%s', 'hi');
  my_sprintf("%s", 'hi');
}
