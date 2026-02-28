<?hh

<<__EntryPoint>>
function test() :mixed{
  if (__hhvm_intrinsics\launder_value(true)) {
    include '685-1.inc';
  } else {
    include '685-2.inc';
  }

  include '685-classes.inc';

  $vars = get_class_vars('A');
  uasort(inout $vars,  HH\Lib\Legacy_FIXME\cmp<>);
  var_dump($vars);
  $vars = get_class_vars('B');
  uasort(inout $vars,  HH\Lib\Legacy_FIXME\cmp<>);
  var_dump($vars);
  $vars = get_class_vars('C');
  uasort(inout $vars,  HH\Lib\Legacy_FIXME\cmp<>);
  var_dump($vars);
  $vars = get_class_vars('X');
  uasort(inout $vars,  HH\Lib\Legacy_FIXME\cmp<>);
  var_dump($vars);
  $vars = get_class_vars('Y');
  uasort(inout $vars,  HH\Lib\Legacy_FIXME\cmp<>);
  var_dump($vars);
  $vars = get_class_vars('Z');
  uasort(inout $vars,  HH\Lib\Legacy_FIXME\cmp<>);
  var_dump($vars);
}
