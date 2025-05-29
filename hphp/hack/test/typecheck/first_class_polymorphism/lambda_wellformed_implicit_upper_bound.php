<?hh
<<file: __EnableUnstableFeatures('polymorphic_lambda')>>

class A<T> {}

function wellformed_implicit_upper_bound(): void {
  $_ = function<T>(A<T> $_): void ==> {};
}
