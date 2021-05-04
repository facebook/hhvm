<?hh

interface I { const type T = int; }

abstract class A implements I {
  // error, abstract cannot override concrete
  abstract const type T = string;
}

interface I1 extends I { // same error
  abstract const type T = bool;
}

trait T1 implements I { // same error
  abstract const type T = float;
}

trait T2 { // same error
  require implements I;
  abstract const type T = float;
}
