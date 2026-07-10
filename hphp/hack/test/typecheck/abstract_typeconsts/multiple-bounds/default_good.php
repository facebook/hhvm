<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>

// Multiple bounds with a default: the default must satisfy every `as` bound and
// sit above every `super` bound.

interface UA {}
interface UB {}
class L implements UA, UB {}
class L1 extends L {}
class L2 extends L {}

abstract class C {
  abstract const type TA as UA as UB super L1 super L2 = L;

  public function take(this::TA $_): void {}
}

// A subclass that does not override picks up the default.
class UsesDefault extends C {}

function use_it(UsesDefault $u, L $l): void {
  $u->take($l);
}
