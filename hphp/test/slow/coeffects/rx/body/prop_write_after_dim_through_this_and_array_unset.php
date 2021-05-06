<?hh

class C {
  public $p;

  public function bad()[rx] {
    $io = new stdClass();
    $this->p = darray[2 => $io];

    unset($this->p[2]->q);
  }
}

<<__EntryPoint>>
function test() {
  $c = new C();
  $c->bad();
}
