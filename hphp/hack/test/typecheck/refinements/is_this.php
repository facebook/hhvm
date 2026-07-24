<?hh

class C {
  public function test(mixed $x): void {
    if ($x is this) {
      hh_show($x);
    } else {
      hh_show($x);
    }
  }

  public function test_self(this $x): void {
    if ($x is this) {
      hh_show($x);
    } else {
      hh_show($x);
    }
  }
}
