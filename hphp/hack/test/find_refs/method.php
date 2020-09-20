<?hh

class C {
  public function foo() {
    $this->foo();
  }
}

class D extends C {
  public function f() {
    $this->foo();
  }
}

type E = D;

function test(C $c, D $d, E $e) {
  $c->foo();
  $d->foo();
  $e->foo(); //TODO(?): this will say D::foo(). Should it say E::foo() instead?

  inst_meth($c, 'foo');
  inst_meth($d, 'foo');
  inst_meth($e, 'foo');

  meth_caller('C', 'foo');
  meth_caller('D', 'foo');
  meth_caller('E', 'foo'); // TODO: This is not detected
}
