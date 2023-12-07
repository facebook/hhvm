<?hh

interface I {
}

abstract class A {
  abstract const type T;
}

class AI extends A { const type T = int; }
class AJ extends A { const type T = string; }
class AK extends A { const type T = float; }

class C<T, -TWriteValue> implements I {}

enum class E: I {
  C<AI, int> AI = new C();
  C<AJ, string> AJ = new C();
  C<AK, float> AK = new C();
  C<string, num> S = new C();
}

function f<TWriteValue, TSettings as A with { type T as arraykey }, TField as C<TSettings, TWriteValue>>(
  HH\EnumClass\Label<E, TField> $label,
  TWriteValue $value,
): void {}

function g(): void {
    f(#AUTO332
}
