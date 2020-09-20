<?hh // strict

// Test of expression dependent type of local variable for try/catch/finally

abstract class C {
  abstract const type T as num;

  abstract public function get(): this::T;
  abstract public function get_a_C(): C;

  public function test(C $c): void {
    // New stable id
    hh_show($c->get());
    hh_show($c->get());

    try {
      // same id
      hh_show($c->get());
      $c = $c->get_a_C();
      // new id
      hh_show($c->get());
    } catch (Exception $e) {
      // new id
      hh_show($c->get());
    } finally {
      // new id
      hh_show($c->get());
    }

    // FIXME: this should have the same id as finally
    hh_show($c->get());
  }
}
