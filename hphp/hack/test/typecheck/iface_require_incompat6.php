<?hh

class Super<T as num> {}

interface I1 {
  require extends Super<float>;
}

trait T1 implements I1 {
  require extends Super<int>;
}
