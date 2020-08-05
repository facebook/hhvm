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

// On its own this is not an error
interface I3 extends I1, I2 {}

// But as soon as it is used, we fail to extend Super<float>
class C extends Super<int> implements I3 { }
