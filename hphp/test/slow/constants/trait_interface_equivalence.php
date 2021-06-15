<?hh

interface I1 {
  const type T = int;
}

trait T2 {
  const type T = string;
}

// behavior should be equivalent to if T2 was `interface I2`
// and the declaration was `class C implements I1, I2`
class C implements I1 {
  use T2;
}
