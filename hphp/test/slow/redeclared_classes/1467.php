<?hh

function f($i) {
  $j = 1;
  var_dump($j);
  if ($i == 1) {
    include '1467-1.inc';
  }
 else {
    include '1467-2.inc';
 }
}
if ($i == 1) {
  include '1467-3.inc';
}
f(1);
$obj = new p();
var_dump($obj);
$obj = new c();
var_dump($obj);
