<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>

// Multiple bounds under `everything_sdt`: every class is implicitly
// `<<__SupportDynamicType>>`, and abstract type constants get an implicit
// `supportdyn<mixed>` upper bound. The explicit multiple `as`/`super` bounds
// must coexist with that implicit bound.

interface UA {}
interface UB {}
class Both implements UA, UB {}

abstract class C {
  abstract const type TA as UA as UB super Both;

  public function take(this::TA $_): void {}
}

class Impl extends C {
  const type TA = Both;
}

function use_it(Impl $i, Both $b): void {
  $i->take($b);
}

// A self-referential upper bound is dropped by cycle detection; the remaining
// `as UA` bound stays, alongside the implicit supportdyn bound under SDT.
abstract class SelfRef {
  abstract const type TA as UA as this::TA;

  public function take(this::TA $_): void {}
}
