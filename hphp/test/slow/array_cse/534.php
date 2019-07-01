<?hh

function f(array $a = null, $e) {
  if (is_int($e) || is_string($e)) {
    $a[$e] ??= array();
    $a[$e]['foo'] = 30;
    try { $x = $a[$e]['baz']; } catch (Exception $e) { echo $e->getMessage()."\n"; $x = null; }
  } else {
    $a[$e] = 30;
    try { $x = $a[$e]; } catch (Exception $e) { echo $e->getMessage()."\n"; $x = null; }
    $a[$e] = 50;
  }
  var_dump($a, $x);
}
function g($x, $y) {
  $x[$y]['foo'] = 30;
  $x[$y]['bar'] = 30;
  $z = $x[$y]['bar'];
  $x[$y]['baz'] = 30;
  if ($z) {
    $x[$y]['baz'] = 30;
  }
  return $x;
}
function h($x, $y) {
  if ($x) {
    $x[$y]['foo'] = 30;
    $x[$y]['bar'] = 30;
  }
  try { var_dump($x[$y]['foo']); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}

<<__EntryPoint>>
function main_534() {
f(array(), 'e');
f(array('e' => array('baz' => 40)), 'e');
var_dump(f(array('y' => array()), 'y'));
var_dump(f(array(), 'y'));
var_dump(f(array(), array()));
h(array(), 0);
h(array(array()), 0);
}
