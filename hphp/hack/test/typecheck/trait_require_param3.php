<?hh

class Super<T> {}

trait T1 {
  require extends Super<int>;
}

trait T2 {
  require extends Super<float>;
}

class C<T> extends Super<T> {
  use T1;
  use T2;
}
