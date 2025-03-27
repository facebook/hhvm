<?hh

final class F {
  const type T = int;
}

interface I<T> {
  public static function gen(): void;
}

abstract class A implements I<this::TInner::T> {
  abstract const type TInner as F;
}

abstract final class C extends A {
  const type TInner = F;

  <<__Deprecated("DO NOT CALL")>>
  public static function gen(): void {}
}

function test(classname<I<int>> $cn = C::class): void {
  $cn::gen();
}
