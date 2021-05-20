<?hh

abstract class A {
  const type T = float;
}
interface I {
  const type T = int;
}

// still expect fatal here
class C extends A implements I {}
