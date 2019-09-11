<?hh


$SOME_VAR = 'foo';
function f($a0, $a1, $a2, $a3) {
  var_dump($a0['SOME_VAR'], $a1, $a2, $a3);
}
function g($a0, $a1, $a2, $a3) {
  var_dump($a0['SOME_VAR'], $a1, $a2, $a3);
}
function h($fcn) {

  $fcn($GLOBALS['GLOBALS'], $GLOBALS['_POST'], Yield2171::$x, Yield2171::$x++);
  yield 64;
}
foreach (h(rand(0, 1) ? 'f' : 'g') as $v) {
  var_dump($v);
}

abstract final class Yield2171 {
  public static $x = 32;
}
