<?hh

class A {
  const int X = 3;
}
class B extends A {
  const int X = 4;
}

trait T {
  require extends A;
}
class C extends B {
  use T;
}
