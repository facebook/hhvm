<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>
<<file:__EnableUnstableFeatures('union_intersection_type_hints')>>

abstract class A {
  abstract const type Ti as int;
  abstract const type Ta as arraykey;
  abstract const type Tia as arraykey as int;
  abstract const type Tsi as (string & int);
}

abstract class AGoodOverride extends A {
  abstract const type Ti as int as num; // OK: <:int subsumes <:num
  abstract const type Ta as string; // OK: <:string is tighter
  abstract const type Tia as int; // OK: <:int subsumes <:arraykey in parent
  abstract const type Tsi as string as int; // OK: child desugars to `as (string & int)`
}

abstract class AGoodOverride2 extends A {
  abstract const type Tia as (arraykey & int); // OK: parent desugars to `as (array & int)`
}

abstract class ABadOverride extends A {
  abstract const type Ti as string; // ERROR: string </: int
  abstract const type Ta as mixed; // ERROR: mixed </: arraykey
  abstract const type Tia as string as arraykey; // ERROR: (string & arraykey) </: (string & int)
  abstract const type Tsi as arraykey; // ERROR: arraykey </: (string & int)
}
