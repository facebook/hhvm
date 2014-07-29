<?hh

class Super<T> {}

interface I1 {
  require extends Super<int>;
}

interface I2 {
  require extends Super<float>;
}

interface I3 extends I1, I2 {}
