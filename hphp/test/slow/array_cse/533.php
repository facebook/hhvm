<?hh

function f($x, $y) {
  var_dump($x[$y]);
  if ($x[$y]) print "HI\n";
}
function g(&$x, $y) {
  var_dump($x[$y]);
  if ($x[$y]) print "HI\n";
}

<<__EntryPoint>>
function main_533() {
f(null, 0);
f(array(0), 0);
f(array(0), 'noidx');
f('abc', 0);
f('abc', 'noidx');
g($x = null, 0);
g($x = array(0), 0);
g($x = array(0), 'noidx');
g($x = 'abc', 0);
g($x = 'abc', 'noidx');
}
