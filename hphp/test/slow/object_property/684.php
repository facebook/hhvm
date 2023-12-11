<?hh

function f() :mixed{
  return false;
}
class B {
  public $a = dict[1 => 1];
  public static $b = vec[1, 2, 3];
}

<<__EntryPoint>>
function main() :mixed{
  if (f()) {
    include '684-1.inc';
  } else {
    include '684-2.inc';
  }
  $vars = get_class_vars('A');
  ksort(inout $vars);
  var_dump($vars);
  A::$a = 1;
  $vars = get_class_vars('A');
  ksort(inout $vars);
  var_dump($vars);
  $vars = get_class_vars('B');
  ksort(inout $vars);
  var_dump($vars);
}
