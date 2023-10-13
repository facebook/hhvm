<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {
  public function f(): void;
}

final class C<T as I> {
  public static function g(T $x): void {
    $x->f();
  }
}

function test(int $x): void {
  $c = C::class;
  $c::g($x);
}
