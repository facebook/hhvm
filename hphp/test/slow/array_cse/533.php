<?hh

function f($x, $y) {
  try {
    var_dump($x[$y]);
    if ($x[$y]) print "HI\n";
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
function g(inout $x, $y) {
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
  $x = null; g(inout $x, 0);
  $x = array(0); g(inout $x, 0);
  $x = array(0); g(inout $x, 'noidx');
  $x = 'abc'; g(inout $x, 0);
  $x = 'abc'; g(inout $x, 'noidx');
}
