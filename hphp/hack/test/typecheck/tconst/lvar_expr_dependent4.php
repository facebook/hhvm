<?hh

// Test of expression dependent type of local variable for foreach

abstract class C {
  abstract const type T as num;

  abstract public function get(): this::T;
  abstract public function get_a_C(): C;

  public function test(C $k, C $v, KeyedTraversable<C, C> $map): void {
    // New stable id
    hh_show($k->get());
    hh_show($k->get());
    // New stable id
    hh_show($v->get());
    hh_show($v->get());

    foreach ($map as $k => $v) {
      // New ids within the loop
      hh_show($k->get());
      hh_show($v->get());
    }

    // New ids outside of loop
    hh_show($k->get());
    hh_show($v->get());
  }
}
