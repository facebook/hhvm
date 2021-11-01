<?hh

class Super<T as num> {}

trait T1 {
  require extends Super<int>;
}

trait T2 {
  require extends Super<float>;
}

class C extends Super<int> {
  use T1;
  use T2;
}
