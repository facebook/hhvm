<?hh

// This file test the expression dependent type associated with local variables

interface I {
  require extends C;
}

abstract class C {
  abstract const type T as num;

  abstract public function get(): this::T;
  protected function set(this::T $x): void {}

  abstract public function get_a_C(): C;

  public function test(C $c): void {
    // $c has an expression id associated with it used for generating
    // expression dependent types. It is stable.
    hh_show($c->get());
    hh_show($c->get());

    $c->set($c->get());

    // Assigning $c to another local variable doesn't transfer the expression id
    $c_alias = $c;
    $c->set($c_alias->get());
    $c_alias->set($c->get());
    hh_show($c_alias->get());

    // type test do not change the expression id, though the type changes
    invariant($c_alias is I, '');
    hh_show($c_alias);
    hh_show($c_alias->get());

    // Assigning $c a new value creates a new expression id, so future
    // expression dependent types will be incompatible with previous ones
    $c_num = $c->get();
    $c->set($c_num);
    $c = $this->get_a_C();
    hh_show($c->get());
    $c->set($c_num);
  }
}
