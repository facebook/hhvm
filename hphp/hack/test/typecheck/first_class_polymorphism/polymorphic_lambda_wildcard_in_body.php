<?hh
<<file: __EnableUnstableFeatures('polymorphic_lambda')>>

interface I<T> {}

class A<T> implements I<T> {}

function test(): void {
  $f = function<T>(I<T> $x): bool ==> $x is A<_>;

  $g = function<T>(I<T> $x): void {
    if ($x is A<_>) {
      $_ = function<T1>(T1 $_, A<_> $_): void {};
    }
  };
}
