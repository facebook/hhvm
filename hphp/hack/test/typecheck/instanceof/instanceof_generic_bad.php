<?hh

class T {
  public function f<T>(mixed $a): bool {
    return $a is T;
  }
}
