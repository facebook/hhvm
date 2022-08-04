<?hh

class Z {}

trait T1 {
  require extends Z;
}

class A {}

class B extends A {}

trait T {
  require extends B;
  use T1;
}
