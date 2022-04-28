<?hh

trait T1 {
  const int X = 4;
  const type T = int;
}
trait T2 {
  const int X = 5;
  const type T = int;
}

class A {
  use T1;
}
class B extends A {
  use T2;
  const int X = 6;
  const type T = int;
}

class C {
  use T1, T2;
}
