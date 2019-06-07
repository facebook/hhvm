<?hh

function foo() {

  HhbbcPseudomain001::$x = 'yep';
  return 'hehehe';
}

function al() { return 2; }
HhbbcPseudomain001::$x = al();
var_dump(foo());
var_dump(HhbbcPseudomain001::$x);

abstract final class HhbbcPseudomain001 {
  public static $x;
}
