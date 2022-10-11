<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

class Covar<+T> {
  public function __construct(T $_) {}
}

abstract class Box {
  abstract const type T;
}

function f<reify T>(T $a): void {}
function f_nonreified<T>(T $a): void {} // OK

function g(int $x, Box with { type T = int } $b): void {
  f<int>($x); // OK
  f_nonreified<Box with { type T = int }>($b); // OK

  f<Box with { type T = int }>($b); // ERROR

  $covar_b = new Covar($b);
  f<Covar<Box with { type T = int }>>($covar_b); // ERROR
}

class C<reify T> {}
class C2<reify T> extends C<T> {}

class D extends C<Covar<Box with { type T = int }>> {} // ERROR (nested)

function in_bound_only<
  reify T as Box with { type T = int } // OK (in bound only)
>(T $x): void {}

class Ok<reify T as Box with { type T = int }> {} // OK (in bound only)
