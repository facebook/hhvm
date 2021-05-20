<?hh

interface I1 {
  const type T = float;
}
interface I2 {
  const type T = int;
}

// still expect fatal here
class C implements I1, I2 {}
