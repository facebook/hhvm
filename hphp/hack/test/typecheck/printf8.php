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

interface PlainSprintf {
  public function format_s<T>(T $s): string;
}

function sprintf(FormatString<PlainSprintf> $f, ...): string {
  return 'hi';
}

function f() {
  sprintf('%s', 'hi');
  sprintf("%s", 'hi');
}
