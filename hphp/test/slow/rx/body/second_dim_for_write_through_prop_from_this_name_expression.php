<?hh

class C {
  public $p;

  <<__Rx>>
  public function bad() {
    $p1 = new stdClass();
    $p1->q = darray[3 => true];
    $this->p = $p1;

    $this->p->{__hhvm_intrinsics\launder_value('q')}[3] = false;
  }
}

<<__EntryPoint>>
function test() {
  $c = new C();
  $c->bad();
}
