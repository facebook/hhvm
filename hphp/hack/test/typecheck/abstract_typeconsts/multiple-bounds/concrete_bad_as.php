<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>

interface UA {}
interface UB {}
class OnlyA implements UA {}

abstract class C {
  abstract const type TA as UA as UB;
}

class Bad extends C {
  // `OnlyA` satisfies `as UA` but not `as UB`.
  const type TA = OnlyA;
}
