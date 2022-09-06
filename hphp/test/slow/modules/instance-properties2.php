<?hh
<<file:__EnableUnstableFeatures("modules")>>


<<__EntryPoint>>
function main(): void {
  include "module.inc";
   include "instance-properties.inc";
  ok();
  $x = new Foo(vec[]);
  $z = $x->x; // error, cannot get internal property
}
