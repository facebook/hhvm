<?hh
// Minimal POC extracted from actual Ent design issues

interface I {
}

abstract class A {
  abstract const type T;
}

class AI extends A { const type T = int; }
class AJ extends A { const type T = string; }
class AK extends A { const type T = float; }

class C<T> implements I {}

enum class E: I {
  C<AI> A = new C();
  C<AJ> S = new C();
  C<AK> F = new C();
}

function f<Tinner as A with { type T as arraykey }, T as C<Tinner>>(
  HH\EnumClass\Label<E, T> $label,
): void {}

function g(): void {
  f(#AUTO332
}
