<?hh

class It { public $x = 1; }
<<__EntryPoint>>
function main_g9() :mixed{
$it = new It;
$foo = function() use($it) {
  $it->x += 10;
  yield $it->x;
  $it->x += 100;
  yield $it->x;
  $it->x += 1000;
  yield $it->x;
};
$x = $foo();
$y1 = clone $x;
$y2 = clone $x;
foreach ($x as $v) {
  echo $v . "\n";
}
echo "========\n";
foreach ($y1 as $v) {
  echo $v . "\n";
}
echo "========\n";
foreach ($y2 as $v) {
  echo $v . "\n";
}
}
