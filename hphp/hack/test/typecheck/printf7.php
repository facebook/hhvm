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

interface MyPlainSprintf {
  public function format_s(mixed $s): string;
}

function my_sprintf(HH\FormatString<MyPlainSprintf> $f, mixed ...$_): string {
  return 'hi';
}

function f(): void {
  my_sprintf('%s', 'hi');
  my_sprintf("%s", 'hi');
}
