<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public static function f1(): void {}
  public static function f2(): void {}
  public static function f3(int $_): int {
    return 0;
  }
}
interface I {
  public static function f2(int $_): void;
  public static function f3(arraykey $_): string;
}

function foo(I $c): void {
  if ($c is A) {
    $c::f1();
    $c::f2();
    $c::f2(1);
    $x = $c::f3(0);
    expect<int>($x);
    expect<string>($x);
    $y = $c::f3("");
    expect<string>($y);
    expect<int>($y); // error
  }
}

function expect<T>(T $x): void {}
