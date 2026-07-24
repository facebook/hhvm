<?hh

class C<T> {
  public function test(mixed $x): void {
    if ($x is this) {
      hh_show($x);
    } else {
      hh_show($x);
    }
  }
}
