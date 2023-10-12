<?hh // strict

// This test that expression display ids are properly reset
// All hh_shows should report the same output
abstract class C {
  abstract const type T as num;

  abstract public function get(): this::T;

  public function meth1(C $c): void {
    hh_show($c->get());
  }

  public function meth2(C $c): void {
    hh_show($c->get());
  }
}

function fun1(C $c): void {
  hh_show($c->get());
}

function fun2(C $c): void {
  hh_show($c->get());
}
