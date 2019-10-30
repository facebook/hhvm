<?hh

if (!isset($h)) {
  if (isset($g)) {
    include '1487-1.inc';
  }
  else {
    include '1487-2.inc';
  }
}
else {
  if (isset($g)) {
    include '1487-3.inc';
  }
  else {
    include '1487-4.inc';
  }
}
abstract class B implements A {
  function bar() {
  }
}
var_dump(get_class_methods('A'));
var_dump(get_class_methods('B'));
var_dump(get_class_methods('X'));
var_dump(get_class_methods('Y'));
