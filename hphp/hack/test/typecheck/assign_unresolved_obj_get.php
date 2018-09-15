<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<T> {
  public function __construct(public T $value) {}
}

function test(bool $b, int $x, float $y, num $z): void {
  if ($b) {
    $xory = $x;
  } else {
    $xory = $y;
  }
  $c = new C($xory);
  $c->value = $z;
}
