<?hh

// typical of code written before traits could declare constants
interface I1 {
  const int X = 4;
}
trait T1 implements I1 {}
interface I2 {
  const int X = 5;
}
trait T2 implements I2 {}
interface I3 {
  const int X = 6;
}
trait T3 implements I3 {}
class C {
  use T1, T2, T3;
}
