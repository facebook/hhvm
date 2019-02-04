<?hh

abstract class C {
  <<__Enforceable>>
  abstract const type T as num;

  abstract const type Tu as num;

  <<__Enforceable>>
  const type Tv = int;

  const type Tw = int;
}
