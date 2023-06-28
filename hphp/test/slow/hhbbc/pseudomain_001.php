<?hh

function foo() :mixed{

  HhbbcPseudomain001::$x = 'yep';
  return 'hehehe';
}

function al() :mixed{ return 2; }

abstract final class HhbbcPseudomain001 {
  public static $x;
}
<<__EntryPoint>>
function entrypoint_pseudomain_001(): void {
  HhbbcPseudomain001::$x = al();
  var_dump(foo());
  var_dump(HhbbcPseudomain001::$x);
}
