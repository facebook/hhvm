<?hh

class C {
  public function f(mixed $m, dynamic $d): void {
    $m;
    $d;
    $m;
    $d;
    if ($m is nonnull) {
      $m;
    }
  }
}
