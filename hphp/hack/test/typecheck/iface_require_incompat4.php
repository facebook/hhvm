<?hh // partial

class Super<T> {
  public ?T $x;
}

interface I1 {
  require extends Super<int>;
}

interface I2 {
  require extends Super<float>;
}

interface I3 extends I1, I2 {}
