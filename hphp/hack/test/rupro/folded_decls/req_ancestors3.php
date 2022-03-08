<?hh

trait T1 {
  require extends B;
}

class A {}

class B extends A {}

trait T {
  require extends B;
  use T1;
}
