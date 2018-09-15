<?hh // strict

class C {

  public function baz(dynamic $x): void {}

}

function foo(dynamic $x): void {}

function bar(): void {
  $c = new C();
  $c->baz(5);
  foo('hi');
}
