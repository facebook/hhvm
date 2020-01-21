<?hh

class C {
  public $p;

  <<__Rx>>
  public function bad() {
    $this->p = new stdClass();

    unset($this->p->q);
  }
}

<<__EntryPoint>>
function test() {
  $c = new C();
  $c->bad();
}
