<?hh

interface I1 {
  abstract const type T = int;
}

interface I2 extends I1 {
  const type T = string;
}

class child implements I2, I1 {}
