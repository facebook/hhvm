<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>

// A child may tighten the `as` bound and widen the `super` bound.

interface UA {}
interface UB {}
interface UC {}
class M implements UA, UB, UC {}
class Lo extends M {}
class Lo2 extends M {}

abstract class P {
  abstract const type TA as UA as UB super Lo;
}

abstract class NarrowAs extends P {
  // OK: `as` is tighter (adds UC), `super` unchanged.
  abstract const type TA as UA as UB as UC super Lo;
}

abstract class WidenSuper extends P {
  // OK: `super` is wider (parent `Lo` <: child `Lo | Lo2`).
  abstract const type TA as UA as UB super Lo super Lo2;
}
