<?hh

<<file:__EnableUnstableFeatures('case_types')>>

class A {}
case type C = A | int;

function foo(C $x): C { return $x; }

<<__EntryPoint>>
function main():void {
  require "test.inc";
  throw_errors();

  foo(new A);
  foo(1);
  foo("ok");
  foo(1.1);

  var_dump(type_structure_for_alias('C'));
}
