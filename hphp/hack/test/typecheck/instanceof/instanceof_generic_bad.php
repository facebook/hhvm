<?hh

class T {
  public function f<T>($a): bool {
    return $a instanceof T;
  }
}
