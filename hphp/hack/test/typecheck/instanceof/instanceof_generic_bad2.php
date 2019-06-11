<?hh // partial

class T<T> {
  public function f($a): bool {
    return $a is T;
  }
}
