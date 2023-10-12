<?hh // strict

interface I1 {
  abstract const type T as mixed;
}
interface I2 {
  abstract const type T as arraykey;
}

interface I extends I1, I2 {}
