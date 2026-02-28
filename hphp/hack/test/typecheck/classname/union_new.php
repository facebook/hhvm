<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__ConsistentConstruct>>
class C {
  public function __construct(int $s) {}
}
final class D {
  public function __construct(int $f) {}
}
interface I {}

function test_it(bool $b): void {
  if ($b) {
    $t = C::class;
  } else {
    $t = D::class;
  }
  $x = new $t(2);
}

function main(): void {
  test_it(true);
}
