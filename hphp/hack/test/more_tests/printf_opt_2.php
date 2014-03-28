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

newtype FormatString<T> = string;

interface Dummy {
  public function format_s(string $s) : string;
}

function takesFS(?FormatString<Dummy> $f = null) : void {
}
function _() : void {
  takesFS('%s', 1);
}
