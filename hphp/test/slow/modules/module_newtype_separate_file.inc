<?hh
<<file:__EnableUnstableFeatures("modules")>>
module foo;
function separate_file(Foo $x) : void {
  $x->foo(); // ok
}
