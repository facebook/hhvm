<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

function expecting<T>(T $_): void {}

class A {}

class B {

  const type T6 = F::T1;
  const type T7 = A;
}

class C {
  const type T5 = B;
}

abstract class D {
  const type T4 = C;
  const type T3 = this::T4::T5;
}

abstract class E {
  const type T2 = D;
}

class F {
  const type T1 = E::T2::T3;
  public static function foo(
    this $_,
    this::T1 $_,
    this::T1::T6 $_,
    this::T1::T7 $_,
  ): void {}
}

function refIt(): void {
  $fptr = F::foo<>;
  expecting<
    HH\FunctionRef<(readonly function(F with { type T1 = B }, B, B, A): void)>,
  >($fptr);
}
