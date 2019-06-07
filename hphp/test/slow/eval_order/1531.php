<?hh

class B {
}
function __autoload($c) {
  var_dump($c);
  include '1531.inc';
}

<<__EntryPoint>>
function main_1531() {
var_dump(class_parents("A", false));
var_dump(class_parents("A"));
var_dump(class_exists("A"));
}
