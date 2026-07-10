<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>
<<file:__EnableUnstableFeatures('union_intersection_type_hints')>>

interface Huge {}
interface Bigly extends Huge {}
interface Mid extends Bigly {}
interface Smol extends Mid {}
interface Teeny extends Smol {}

interface UpperOne {
  abstract const type TB as Huge;
}
interface UpperTwo {
  abstract const type TB as Bigly;
}

// Each lower bound implements both upper bounds, so the union of the super
// bounds is a subtype of the intersection of the as bounds.
interface LowerOne extends UpperOne, UpperTwo {
  abstract const type TB as Bigly super Smol;
}
interface LowerTwo extends UpperOne, UpperTwo {
  abstract const type TB as Bigly super Teeny;
}

function takes_one<T as UpperOne super LowerOne>(T $x): T {
  return $x;
}

function takes_two<T as UpperTwo super LowerTwo>(T $x): T {
  return $x;
}

function not_too_bigly_not_too_smol<T as Bigly super Smol>(T $x): T {
  return $x;
}

abstract class Home {
  abstract const type TA as UpperOne as UpperTwo super LowerOne super LowerTwo;

  public function check(this::TA $ta, this::TA::TB $tb): void {
    // Intersection of the as bounds and union of the super bounds.
    takes_one($ta);
    takes_two($ta);

    // Intersection and union applied through nested access.
    not_too_bigly_not_too_smol($tb);
  }
}

function refIt(): void {
  $f = meth_caller(Home::class, 'check');
  hh_expect<
    HH\FunctionRef<(readonly function<
      TA1 as (
        UpperOne with { type TB = TB1 } &
        UpperTwo with { type TB = TB1 }
      ) super (
        LowerOne with { type TB = TB1 } |
        LowerTwo with { type TB = TB1 }
      ),
      TB1 as Bigly super Smol,
    >(Home with { type TA = TA1 }, TA1, TB1): void)>,
  >($f);
}
