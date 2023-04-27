<?hh

class C {
  public function m(string $s): void {}
}

function main(C $c, dynamic $d): void {
  $c->m($d);
}
