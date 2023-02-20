<?hh

class C {
  public function m(string $s): void {}
}

function main(C $c): void {
  $c->m("hello");
}
