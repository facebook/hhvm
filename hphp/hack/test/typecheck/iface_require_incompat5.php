<?hh

class Super<T as num> {}

interface I1 {
  require extends Super<int>;
  require extends Super<float>;
}
