<?hh

class A {
  const type T = int;
}

interface I {
  const type T = int;
}

class B extends A implements I {}
