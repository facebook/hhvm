<?hh

class C {
  public function camelCase(): void {}
}

function foo(): void {
  $c = new C();
  $c->camelcase();
}
