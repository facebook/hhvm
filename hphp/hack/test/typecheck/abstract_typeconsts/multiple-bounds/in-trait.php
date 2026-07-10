<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>

// Multiple bounds on a type constant declared in a trait.

interface UA {}
interface UB {}
class Both implements UA, UB {}

trait T {
  abstract const type TA as UA as UB super Both;
  public function take(this::TA $_): void {}
}

class Uses {
  use T;
  const type TA = Both;
}

function use_it(Uses $u, Both $b): void {
  $u->take($b);
}
