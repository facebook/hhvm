<?hh

abstract class A { const type T = int; }

abstract class B extends A {
  // error, abstract cannot override concrete
  abstract const type T = string;
};

interface I { // same error
  require extends A;
  abstract const type T = bool;
}

trait T { // same error
  require extends A;
  abstract const type T = float;
}
