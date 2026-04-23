<?hh
<<file:__EnableUnstableFeatures('with_refinement_alias')>>
class Covar<+T> {
  public function __construct(T $_) {}
}

abstract class Box {
  abstract const type T;
}

function f<reify T>(T $a): void {}
function f_nonreified<T>(T $a): void {} // OK
type BoxWith = Box with { type T = int };
function g(int $x, BoxWith $b): void {
  f<int>($x); // OK
  f_nonreified<BoxWith>($b); // OK

  f<BoxWith>($b); // ERROR

  $covar_b = new Covar($b);
  f<Covar<BoxWith>>($covar_b); // ERROR
}

class C<reify T> {}
class C2<reify T> extends C<T> {}

class D extends C<Covar<BoxWith>> {} // ERROR (nested)

function in_bound_only<
  reify T as BoxWith, // OK (in bound only)
>(T $x): void {}

class Ok<reify T as BoxWith> {} // OK (in bound only)
