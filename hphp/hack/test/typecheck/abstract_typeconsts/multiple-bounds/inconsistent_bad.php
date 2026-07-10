<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>
<<file:__EnableUnstableFeatures('union_intersection_type_hints')>>

interface UpperOne {}
interface UpperTwo {}

// LowerOne only implements UpperOne and LowerTwo only implements UpperTwo, so
// the union of the super bounds is NOT a subtype of the intersection of the as
// bounds. The type constant is uninhabitable; any use surfaces the error.
interface LowerOne extends UpperOne {}
interface LowerTwo extends UpperTwo {}

abstract class Home {
  abstract const type TA as UpperOne as UpperTwo super LowerOne super LowerTwo;

  public function check(this::TA $_): void {}
}

function refIt(): void {
  $f = meth_caller(Home::class, 'check');
  takes_it($f);
}

function takes_it(
  HH\FunctionRef<(readonly function<
    TA1 as (UpperOne & UpperTwo) super (LowerOne | LowerTwo),
  >(Home with { type TA = TA1 }, TA1): void)> $_,
): void {}
