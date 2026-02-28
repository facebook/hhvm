<?hh

<<file:__EnableUnstableFeatures('type_const_super_bound')>>

abstract class GoodWithExperimental {
  abstract const type T super string;
  abstract const type Tdflt super int = num;
}
