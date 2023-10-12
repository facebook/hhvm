<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public static function f(): void {}
}

function test(bool $b, mixed $x): void {
  if ($b) {
    /* HH_FIXME[4064] */
    $c = $x->p;
  } else {
    $c = C::class;
  }
  $c::f();
}
