<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>

interface UA {}
interface UB {}
class OnlyA implements UA {}
class LowA extends OnlyA {}
class Both implements UA, UB {}
class Hi implements UA, UB {}

abstract class C {
  // Default `OnlyA` satisfies `as UA` but not `as UB`.
  abstract const type TBadAs as UA as UB super LowA = OnlyA;

  // `super Hi` is not below the default `Both`.
  abstract const type TBadSuper as UA as UB super Hi = Both;
}
