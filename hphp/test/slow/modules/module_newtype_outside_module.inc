<?hh

function outside_module(Foo $x) : void {
  $x->foo(); // ok
}
