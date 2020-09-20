<?hh

class C {
  public $p;

  <<__Rx>>
  public function bad() {
    $this->p = new stdClass();

    $this->p->q = 2;
  }
}

<<__EntryPoint>>
function test() {
  $c = new C();
  $c->bad();
}
