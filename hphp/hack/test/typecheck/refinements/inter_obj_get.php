<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {
  public function f2(): void;
  public function f3(arraykey $x): string;
}
class A {
  public function f1(): void {}
  public function f2(int $_): void {}
  public function f3(int $x): int {
    return 0;
  }
}

function foo(I $c): void {
  if ($c is A) {
    $c->f1();
    $c->f2();
    $c->f2(1);
    $x = $c->f3(0);
    expect<int>($x);
    expect<string>($x);
    $y = $c->f3("");
    expect<string>($y);
    expect<int>($y); // error
  }
}

function expect<T>(T $x): void {}
