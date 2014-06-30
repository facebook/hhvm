<?hh

class Super<T as num> {}

interface I1 {
  require extends Super<int>;
}

interface I2 {
  require extends Super<float>;
}

class C extends Super<int>
implements I1, I2 {
}
