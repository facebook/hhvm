<?hh

<<file:__EnableUnstableFeatures('case_types')>>

class A {}
case type C = A | int;

function foo(C $x): C { return $x; }

function handler($errno, $errstr, $errfile, $errline, ...):noreturn {
  throw new Exception($errstr);
}

<<__EntryPoint>>
function main():void {
  set_error_handler(handler<>);

  foo(new A);
  foo(1);
  foo("ok");
  foo(1.1);

  var_dump(type_structure_for_alias('C'));
}
