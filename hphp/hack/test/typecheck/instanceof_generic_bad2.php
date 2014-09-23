<?hh

class T<T> {
  public function f($a): bool {
    return $a instanceof T;
  }
}
