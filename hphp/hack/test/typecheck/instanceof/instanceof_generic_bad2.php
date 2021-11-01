<?hh

class T<T> {
  public function f(mixed $a): bool {
    return $a is T;
  }
}
