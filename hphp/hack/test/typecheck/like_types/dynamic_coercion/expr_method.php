<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function __construct(public (function(int): void) $f) {}
}

function test(C $c, dynamic $d): void {
  $f = $c->f;
  // Can't enforce lambda parameter hints so coercion fails
  $f($d);
}
