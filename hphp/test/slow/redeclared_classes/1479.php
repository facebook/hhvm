<?hh


<<__EntryPoint>>
function main_1479() :mixed{
if (!isset($g2)) {
  include '1479-1.inc';
}
 else {
  include '1479-2.inc';
  var_dump(test::$foo);
}
$x = new test();
$x->bar = 1;
$x->foo = 2;
var_dump($x);
}
