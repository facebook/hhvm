<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>

// Multiple `super` bounds only (no `as` bound): the lower bound is the union
// of the declared supers; the upper bound is `mixed`.

interface Base {}
class Sub1 implements Base {}
class Sub2 implements Base {}

abstract class C {
  abstract const type TA super Sub1 super Sub2;

  // A value of the type constant accepts anything below it; both supers flow in.
  public function take(this::TA $_): void {}
}

// A concrete assignment must be a supertype of every declared super bound.
class DGood extends C {
  const type TA = Base;
}

function use_it(DGood $d, Sub1 $s1, Sub2 $s2): void {
  $d->take($s1);
  $d->take($s2);
}
