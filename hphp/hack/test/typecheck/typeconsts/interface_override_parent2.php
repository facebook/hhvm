<?hh

abstract class A {
  abstract const type T;
}

interface I {
  const type T = int;
}

class B extends A implements I {}
