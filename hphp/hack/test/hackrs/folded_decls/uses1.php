<?hh

trait T {}

trait T2 {
  use T;
}

trait T3 {}

class A {
  use T2, T3;
}
