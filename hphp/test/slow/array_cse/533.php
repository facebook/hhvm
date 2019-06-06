<?hh

function f($x, $y) {
  try {
    var_dump($x[$y]);
    if ($x[$y]) print "HI\n";
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
function g(&$x, $y) {
  try {
    var_dump($x[$y]);
    if ($x[$y]) print "HI\n";
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
}

<<__EntryPoint>>
function main_533() {
  f(null, 0);
  f(array(0), 0);
  f(array(0), 'noidx');
  f('abc', 0);
  f('abc', 'noidx');
  $x = null; g(&$x, 0);
  $x = array(0); g(&$x, 0);
  $x = array(0); g(&$x, 'noidx');
  $x = 'abc'; g(&$x, 0);
  $x = 'abc'; g(&$x, 'noidx');
}
