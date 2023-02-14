<?hh

class C {
  public function foo(): void {
    $this->foo();
  }
}

class D extends C {
  public function f(): void {
    $this->foo();
  }
}

class F extends D {
  public function foo(): void {}
}

class G extends F {}

class Unrelated {
  public function foo(): void {}
}

type E = D;

function test(C $c, D $d, E $e, F $f, G $g, Unrelated $u) {
  $c->foo();
  $d->foo();
  $e->foo(); //TODO(?): this will say D::foo(). Should it say E::foo() instead?
  $f->foo();
  $g->foo();
  $u->foo();

  meth_caller('C', 'foo');
  meth_caller('D', 'foo');
  meth_caller('E', 'foo'); // TODO: This is not detected
  meth_caller('F', 'foo'); // TODO: This is not detected
  meth_caller('G', 'foo'); // TODO: This is not detected
  meth_caller('Unrelated', 'foo'); // TODO: This is not detected
}
