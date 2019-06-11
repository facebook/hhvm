<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public static function f(int $i): void {}
}

function test(dynamic $d): void {
  class_meth(C::class, 'f')($d);
}
