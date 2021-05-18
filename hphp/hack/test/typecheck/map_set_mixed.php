<?hh

function f(mixed $x) : void {
  $m = Map {};
  $m->set($x, $x);
}
