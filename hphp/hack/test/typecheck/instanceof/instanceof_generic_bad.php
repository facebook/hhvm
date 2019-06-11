<?hh // partial

class T {
  public function f<T>($a): bool {
    return $a is T;
  }
}
