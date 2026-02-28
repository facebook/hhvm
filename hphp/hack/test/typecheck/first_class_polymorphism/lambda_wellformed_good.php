<?hh
<<file: __EnableUnstableFeatures('polymorphic_lambda')>>

class A {}

abstract class B {
  abstract const type TA as A;
}

class C<T1 as A, T2 as B with { type TA as T1 }> {}

function wellformed_good(): void {
  $_ = function<T1 as A, T2 as B with { type TA as T1 }>(C<T1, T2> $_): void {};
}
