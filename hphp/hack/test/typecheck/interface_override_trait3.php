<?hh


interface I2 {
  const FOO1 = "interface";
}

trait T1 implements I2 {}

trait T {
  use T1;
}

interface I extends I2 {}

class A implements I {
  use T;
}
