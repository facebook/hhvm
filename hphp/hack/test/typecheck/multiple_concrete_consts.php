<?hh // partial
interface I1 {
  const int X = 7;
}
interface I2 {
  const int X = 5;
}
interface I extends I1, I2 {}
