<?hh

function inline_me($x, $y, inout $z) {
 return ($z = ($x + $y));
 }
function gen($x, $y) {
  $arg = null;
  yield inline_me($x, $y, inout $arg);
  yield $arg;
}

<<__EntryPoint>>
function main_1839() {
foreach (gen(10, 20) as $x) {
 var_dump($x);
 }
}
