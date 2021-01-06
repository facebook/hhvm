<?hh

class C {}

function f(dynamic $d) : void {
  $c = new C();
  $d->m(new C());
  $d->m($c);
  $d->m(vec[$c]);
}
