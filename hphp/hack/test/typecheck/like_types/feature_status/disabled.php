<?hh

function f1(~int $x): ~bool {
  return true;
}

function f2(vec<~int> $x): vec<~bool> {
  return vec[];
}

function gen<T>(): void {}

class C<T> {}

function f3(): void {
  vec<~int>[];
  Vector<~int>{};
  new C<~int>();
  gen<~int>();
}

function f4(mixed $x): void {
  $x is ~int;
  $x as ~int;
}

class C1<T as ~int> {}
