<?hh

class C {
  public $p;

  <<__Rx>>
  public function bad() {
    $io = new stdClass();
    $this->p = darray[2 => $io];

    $this->p[2]->q *= 2;
  }
}

<<__EntryPoint>>
function test() {
  $c = new C();
  $c->bad();
}
