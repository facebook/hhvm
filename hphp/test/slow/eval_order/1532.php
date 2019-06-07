<?hh

trait T {
}
function __autoload($c) {
  var_dump($c);
  include '1532.inc';
}

<<__EntryPoint>>
function main_1532() {
var_dump(class_uses("A", false));
var_dump(class_uses("A"));
var_dump(class_exists("A"));
}
