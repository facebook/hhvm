<?hh

interface I1 {
  const string X = "I1";
}
interface I2 {
  const string X = "I2";
}
trait T1 implements I1 {}
trait T2 implements I2 {}


class B {
  use T1;
}
class A extends B implements I2 {
  use T2;
}
