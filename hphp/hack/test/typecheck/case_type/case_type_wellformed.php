<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type GenericBoundsAreChecked<T> = keyset<T> | dict<T, int>;

case type VarianceIsChecked<-Tin as arraykey, +Tout> =
  | (function(Tin, Tout): int)
  | dict<Tin, Tout>;

// TODO(T150253169) - Error message mentions `newtype`
case type BoundsAreChecked as num = int | float | bool;

// TODO(T150253169) - Should report all bounds that are violated
case type MultiBoundsAreChecked as XHPChild, Stringish, arraykey = string | int | bool;

case type Recursive = vec<?Recursive>;
