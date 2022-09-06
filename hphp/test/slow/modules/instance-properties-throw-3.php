<?hh
<<file:__EnableUnstableFeatures("modules")>>


<<__EntryPoint>>
function main(): void {
  include "module.inc";
   include "instance-properties.inc";
  ok();
  $x = new Foo(vec[]);
  try {
    $x->v[] = 6; // error cannot append to internal prop
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
