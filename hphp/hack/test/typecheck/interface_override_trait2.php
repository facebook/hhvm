<?hh

interface I1 {
  const string BAR = "a";
  const string BAR1 = "two";
  const string BAR2 = "three";
}

trait T1 implements I1 {}

interface I {
  const string BAR = "b";
  const string BAR1 = "one";
  const string BAR2 = "four";
}

trait T implements I {}

class B  {
  use T1;
}

final class A extends B {
  use T;
}
