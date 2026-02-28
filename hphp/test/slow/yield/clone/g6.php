<?hh

class Ref { public int $n; }

<<__EntryPoint>>
function main_g6() :mixed{
$a1 = 1;
$a2 = new Ref();
$a2->n = -999999999;
$foo = function () use ($a1, $a2) {
  $a1 += 10;
  $a2->n += 100;
  yield $a1 * 10000 + $a2->n;
  $a1 += 20;
  $a2->n += 200;
  yield $a1 * 10000 + $a2->n;
  $a1 += 30;
  $a2->n += 300;
  yield $a1 * 10000 + $a2->n;
};
$x = $foo();
$a2->n = 2;
$x->rewind();
$y1 = clone $x;
$y2 = clone $x;
for ($y1->next(); $y1->valid(); $y1->next()) {
  $v = $y1->current();
  $v1 = (int)($v / 10000);
  $v2 = $v % 10000;
  echo $v1 . " " . $v2 . "\n";
  ++$a2->n;
}
echo "--------\n";
var_dump($a1, $a2->n);
echo "========\n";
for ($y2->next(); $y2->valid(); $y2->next()) {
  $v = $y2->current();
  $v1 = (int)($v / 10000);
  $v2 = $v % 10000;
  echo $v1 . " " . $v2 . "\n";
  ++$a2->n;
}
echo "--------\n";
var_dump($a1, $a2->n);
echo "========\n";
for ($x->next(); $x->valid(); $x->next()) {
  $v = $x->current();
  $v1 = (int)($v / 10000);
  $v2 = $v % 10000;
  echo $v1 . " " . $v2 . "\n";
  ++$a2->n;
}
echo "--------\n";
var_dump($a1, $a2->n);
}
