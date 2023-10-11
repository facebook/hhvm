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



interface Dummy {
  public function format_s(string $s) : string;
}

function f(\HH\FormatString<Dummy> $x, mixed...$args) : void {
}

function _(string $x) : void {
  f('', $x);
}
