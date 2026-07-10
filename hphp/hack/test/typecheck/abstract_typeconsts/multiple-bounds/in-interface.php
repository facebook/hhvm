<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>

// Multiple bounds on a type constant declared in an interface.

interface UA {}
interface UB {}
class Both implements UA, UB {}

interface I {
  abstract const type TA as UA as UB super Both;
  public function take(this::TA $_): void;
}

class Impl implements I {
  const type TA = Both;
  public function take(this::TA $_): void {}
}

function use_it(Impl $i, Both $b): void {
  $i->take($b);
}
