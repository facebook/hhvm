<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>

// Asymmetric multiplicities: one `as` with two `super`, and two `as` with one
// `super`. Both must be consistent (union of supers <: intersection of ass).

interface UA {}
interface UB {}
class L1 implements UA, UB {}
class L2 implements UA, UB {}

abstract class OneAsTwoSuper {
  abstract const type TA as UA super L1 super L2;
  public function take(this::TA $_): void {}
}

abstract class TwoAsOneSuper {
  abstract const type TA as UA as UB super L1;
  public function take(this::TA $_): void {}
}

// A concrete assignment must satisfy every `as` bound and sit above every
// `super` bound.
class OneAsTwoSuperImpl extends OneAsTwoSuper {
  const type TA = UA;
}
class TwoAsOneSuperImpl extends TwoAsOneSuper {
  const type TA = L1;
}

function use_them(OneAsTwoSuperImpl $a, TwoAsOneSuperImpl $b, L1 $l1): void {
  $a->take($l1);
  $b->take($l1);
}
