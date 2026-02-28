<?hh

class It { public $x = 1; }
<<__EntryPoint>>
function main_g10() :mixed{
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
$x->rewind();
$y1 = clone $x;
$y2 = clone $x;
for ($x->next(); $x->valid(); $x->next()) {
  echo $x->current() . "\n";
}
echo "========\n";
for ($y1->next(); $y1->valid(); $y1->next()) {
  echo $y1->current() . "\n";
}
echo "========\n";
for ($y2->next(); $y2->valid(); $y2->next()) {
  echo $y2->current() . "\n";
}
}
