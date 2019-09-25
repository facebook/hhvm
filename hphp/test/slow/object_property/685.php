<?hh

if (__hhvm_intrinsics\launder_value(true)) {
  include '685-1.inc';
} else {
  include '685-2.inc';
}
class B extends A {
  public $b0 = 3;
  static $b1 = 4;
  static $b2 = 5;
}
class Y extends X {
  public $y0 = 3;
  static $y1 = 4;
  static $y2 = 5;
}
class C {
  public $c0;
  static $c1 = 1;
  static $c2 = 2;
}
class Z {
  public $z0;
  static $z1 = 1;
  static $z2 = 2;
}

<<__EntryPoint>>
function test() {
  $vars = get_class_vars('A');
  asort(inout $vars);
  var_dump($vars);
  $vars = get_class_vars('B');
  asort(inout $vars);
  var_dump($vars);
  $vars = get_class_vars('C');
  asort(inout $vars);
  var_dump($vars);
  $vars = get_class_vars('X');
  asort(inout $vars);
  var_dump($vars);
  $vars = get_class_vars('Y');
  asort(inout $vars);
  var_dump($vars);
  $vars = get_class_vars('Z');
  asort(inout $vars);
  var_dump($vars);
}
