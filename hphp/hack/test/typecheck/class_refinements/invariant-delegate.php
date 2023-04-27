<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface Invariant<T> {}

interface Box {
  abstract const type T;
  public function get(): this::T;
}

function accepts_sub<TBox as Box>(
  Invariant<TBox> $_
): void {}

function accepts_sub_generic_refinement<
  TBox as Box with { type T = T0 },
  T0
>(Invariant<TBox> $_): void {}

function accepts_sub_int_refinement<
  TBox as Box with { type T = int }
>(Invariant<TBox> $_): void {}

abstract class IntBox implements Box { const type T = int; }

function delegate_int_box(Invariant<IntBox> $inv_int_box): void {
  accepts_sub($inv_int_box); // OK
  accepts_sub_generic_refinement($inv_int_box); // OK
  accepts_sub_int_refinement($inv_int_box); // OK
  accepts_eq_int_refinement($inv_int_box); // ERROR
}

// Note: this should never be transitively usable, consider linting against
function accepts_eq_int_refinement(
  Invariant<Box with { type T = int }> $_
): void {}

function delegate_box(Invariant<Box> $inv_box): void {
  accepts_sub_generic_refinement($inv_box); // ERROR
  // this is because accepts_sub_generic_refinement would
  // have to infer some path-dependant existential type:
  // $inv_box::T
}
