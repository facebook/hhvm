<?hh
<<file:__EnableUnstableFeatures("modules")>>


<<__EntryPoint>>
function main(): void {
  include "module.inc";
  include "instance-properties.inc";
  ok();
  $x = new Foo(vec[]);
  try {
    $x->x++; // error, cannot incdec internal property
  } catch (Exception $e) { echo $e->getMessage()."\n"; }

}
