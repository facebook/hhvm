<?hh



<<__EntryPoint>>
function main(): void {
  include "module.inc";
   include "instance-properties.inc";
  ok();
  $x = new Foo(vec[]);
  try {
    $x->x = 10; // error, cannot set internal property
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
