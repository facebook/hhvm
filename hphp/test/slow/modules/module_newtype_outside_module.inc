<?hh
<<file:__EnableUnstableFeatures("modules")>>
function outside_module(Foo $x) : void {
  $x->foo(); // ok
}
