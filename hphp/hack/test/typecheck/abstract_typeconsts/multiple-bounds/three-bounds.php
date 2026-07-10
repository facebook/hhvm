<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>

// More than two bounds of each kind.

interface UA {}
interface UB {}
interface UC {}
class L implements UA, UB, UC {}
class L2 extends L {}
class L3 extends L2 {}

abstract class C {
  abstract const type TA as UA as UB as UC super L super L2 super L3;
  public function take(this::TA $_): void {}
}

class Impl extends C {
  const type TA = L;
}

function use_it(Impl $i, L $l): void {
  $i->take($l);
}
