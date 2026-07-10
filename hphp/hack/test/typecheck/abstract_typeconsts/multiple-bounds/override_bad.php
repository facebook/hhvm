<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>

// A child may not widen the `as` bound nor narrow the `super` bound.

interface UA {}
interface UB {}
class M implements UA, UB {}
class Lo extends M {}
class Lo2 extends M {}
class VeryLo extends Lo {}

abstract class P {
  abstract const type TA as UA as UB super Lo;
}

abstract class WidenAs extends P {
  // ERROR: `as UA` is not a subtype of the parent's `as UA & UB`.
  abstract const type TA as UA super Lo super Lo2;
}

abstract class NarrowSuper extends P {
  // ERROR: parent's `super Lo` is not below the child's `super VeryLo | Lo2`.
  abstract const type TA as UA as UB super VeryLo super Lo2;
}
