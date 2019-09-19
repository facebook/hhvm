<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public static shape('a' => int) $t = shape('a' => 4);
}

function test(dynamic $d): void {
  hh_show(C::$t);
  C::$t = $d;
}
