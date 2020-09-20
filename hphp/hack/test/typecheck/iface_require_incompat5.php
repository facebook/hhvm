<?hh // partial

class Super<T as num> {}

interface I1 {
  require extends Super<int>;
  require extends Super<float>;
}

class Sub extends Super<num> implements I1 {}
