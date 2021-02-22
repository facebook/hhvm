<?hh

abstract class A {
  abstract const type T = string;
}

interface I {
  const type T = int;
}

class B extends A implements I {}
