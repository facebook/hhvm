<?hh

interface I {
}
function __autoload($c) {
  var_dump($c);
  include '1530.inc';
}

<<__EntryPoint>>
function main_1530() {
var_dump(class_implements("A", false));
var_dump(class_implements("A"));
var_dump(class_exists("A"));
}
