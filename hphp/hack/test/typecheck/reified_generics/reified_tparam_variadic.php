<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function f<reify Tf>(int ...$a): void {}
function g<reify Tg>(int $a, ...$_): void {}

class C {
  public function x<reify Tx>(int ...$a): void {}
  public function y<reify Ty>(int $a, ...$_): void {}

}
