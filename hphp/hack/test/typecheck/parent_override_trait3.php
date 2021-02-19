<?hh

trait T1 {
  const FOO1 = "one";
}

interface I2 {
  const FOO1 = "two";
}

trait T2 implements I2 {}


class B {
  use T2;
}

class A extends B {
  use T1;
}
