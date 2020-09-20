<?hh

enum E: int as int {}

abstract class A {
  /* HH_FIXME[2053] */
  abstract const type T as HH\BuiltinEnum<string>;
}

class C extends A {
  const type T = E;
}
