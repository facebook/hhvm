<?hh



<<__EntryPoint>>
function main(): void {
  include "module.inc";
  include "instance-properties.inc";
  ok();
  $x = new Foo(vec[]);
  $x->x++; // error, cannot incdec internal property
}
