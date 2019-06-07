<?hh

function __autoload($c) {
 var_dump($c);
 }
function f() {
 return false;
 }
function test($c) {
  var_dump(class_exists('A'));
  var_dump(interface_exists('A'));
  var_dump(class_exists('B'));
  var_dump(interface_exists('B'));
  var_dump(class_exists($c));
  var_dump(interface_exists('C'));
}

<<__EntryPoint>>
function main_1477() {
if (f()) {
  include '1477-1.inc';
}
 else {
  include '1477-2.inc';
}
test('C');
}
