<?hh

<<__EntryPoint>>
function test() {
  if (__hhvm_intrinsics\launder_value(true)) {
    include '685-1.inc';
  } else {
    include '685-2.inc';
  }

  include '685-classes.inc';

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
