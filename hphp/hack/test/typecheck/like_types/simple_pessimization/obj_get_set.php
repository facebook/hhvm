<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public shape('a' => int) $t = shape('a' => 4);
}

function test(dynamic $d): void {
  $c = new C();
  hh_show($c->t);
  $c->t = $d;
}
