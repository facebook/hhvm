<?hh

function f() {
  return false;
}
class B {
  public $a = darray[1 => 1];
  static $b = varray[1, 2, 3];
}

<<__EntryPoint>>
function main() {
  if (f()) {
    include '684-1.inc';
  } else {
    include '684-2.inc';
  }
  $vars = get_class_vars('A');
  asort(inout $vars);
  var_dump($vars);
  A::$a = 1;
  $vars = get_class_vars('A');
  asort(inout $vars);
  var_dump($vars);
  $vars = get_class_vars('B');
  asort(inout $vars);
  var_dump($vars);
}
