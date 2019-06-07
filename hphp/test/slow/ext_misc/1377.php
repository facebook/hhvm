<?hh

function __autoload($c) {
  var_dump($c);
}
function test() {
  var_dump(is_subclass_of('C', 'D'));
  var_dump(get_class_methods('C'));
  var_dump(method_exists('C', 'foo'));
  include '1377.inc';
  var_dump(is_subclass_of('C', 'D'));
  var_dump(is_subclass_of('C', 'C'));
}

<<__EntryPoint>>
function main_1377() {
test();
var_dump(class_exists('C'));
}
