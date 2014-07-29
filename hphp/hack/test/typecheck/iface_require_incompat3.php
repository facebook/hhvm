<?hh

class Super<T> {}

interface I1 {
  require extends Super<int>;
}

interface I2 extends I1 {}

class C implements I2 {}
