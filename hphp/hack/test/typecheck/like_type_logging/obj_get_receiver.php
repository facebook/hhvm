<?hh

class C {
  public function m(): void {}
}

function f<T>(T $t): T { return $t; }

function like_c(): void {
  $c = f<C>(new C());
  $c->m();
}

function c(): void {
  $c = new C();
  $c->m();
}
