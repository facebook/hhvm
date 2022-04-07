<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>

abstract class A {
  abstract const type T0;
  abstract const type T1 as string;
  abstract const type T2 as arraykey as int;
  abstract const type T3 as int as arraykey as num = int;
}
