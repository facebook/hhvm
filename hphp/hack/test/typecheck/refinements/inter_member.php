<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public int $x = 0;
}

interface I {
  public function f(int $x): int;
}

class B {
  public string $x = "";
  public function f(string $x): string {
    return $x;
  }
}

function f(A $x): void {
  if ($x is B) {
    expect<int>($x->x);
    expect<string>($x->x);
  }
}

function g(I $x): void {
  if ($x is B) {
    expect<(function (string): string)>(inst_meth($x, 'f'));
    expect<(function (int): int)>(inst_meth($x, 'f'));
    expect<string>($x->f(""));
    expect<int>($x->f(0));
  }
}

function expect<T>(T $_): void {}
