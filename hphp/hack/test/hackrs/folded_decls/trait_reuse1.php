<?hh

trait T {}

class A {
  use T;
}

class B extends A {
  use T;
}
