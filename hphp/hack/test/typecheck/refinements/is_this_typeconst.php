<?hh

abstract class C {
  <<__Enforceable>>
  abstract const type TC as arraykey;

  public function test(mixed $x): void {
    if ($x is this::TC) {
      hh_show($x);
    } else {
      hh_show($x);
    }
  }

  public function test_self(this::TC $x): void {
    if ($x is this::TC) {
      hh_show($x);
    } else {
      hh_show($x);
    }
  }
}
