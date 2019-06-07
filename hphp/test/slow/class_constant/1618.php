<?hh
class X {
  function foo($x = C::FOO, $y = I::FOO) {
}
}
function test() {
  $x = new ReflectionMethod('X', 'foo');
  foreach ($x->getParameters() as $p) {
    var_dump($p->getDefaultValue());
  }
}
function fiz($c) {
  var_dump($c::FOO);
}


<<__EntryPoint>>
function main_1618() {
if (isset($g)) {
  include '1618-1.inc';
} else {
  include '1618-2.inc';
}
fiz('I');
fiz('C');
test();
}
