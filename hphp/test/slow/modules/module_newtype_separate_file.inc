<?hh

module foo;
function separate_file(Foo $x) : void {
  $x->foo(); // ok
}
