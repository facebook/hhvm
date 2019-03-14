<?hh // partial
interface I1 {
  abstract const int X;
}
interface I2 {
  const int X = 5;
}
interface I extends I1, I2 {}
