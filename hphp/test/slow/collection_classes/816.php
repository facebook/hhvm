<?hh

function f($x1, $x2, $x3, $x4) {
  return Vector {
$x1, $x2, $x3, $x4}
;
}
function g($k1, $v1, $k2, $v2) {
  $m = Map {
$k1 => $v1, $k2 => $v2}
;
  return $m;
}
var_dump(f(42, 123.456, 'blah', array(3, 5, 7)));
var_dump(g('foo', 1, 2, 'bar'));
