<?hh

class C {}

function f(): void {
  $c = new C();
  $s = "{$c->x()}";
}
