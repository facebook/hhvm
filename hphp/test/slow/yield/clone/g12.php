<?hh

class Ref {
  function __construct(public $value)[] {}
}
function foo($a, $b, $y) :AsyncGenerator<mixed,mixed,void>{
  $x = $a;
  unset($a);
  unset($b);
  $x += 10;
  $y->value += 100;
  yield $x * 10000 + $y->value;
  $x += 20;
  $y->value += 200;
  yield $x * 10000 + $y->value;
  $x += 30;
  $y->value += 300;
  yield $x * 10000 + $y->value;
}

<<__EntryPoint>>
function main_g12() :mixed{
$a1 = 1;
$a2 = new Ref(-999999999);
$x = foo($a1, $a2, $a2);
$a2->value = 2;
$x->rewind();
$y1 = clone $x;
$y2 = clone $x;
for ($y1->next(); $y1->valid(); $y1->next()) {
  $v = $y1->current();
  $v1 = (int)($v / 10000);
  $v2 = $v % 10000;
  echo $v1 . " " . $v2 . "\n";
  ++$a2->value;
}
echo "--------\n";
var_dump($a1, $a2->value);
echo "========\n";
for ($y2->next(); $y2->valid(); $y2->next()) {
  $v = $y2->current();
  $v1 = (int)($v / 10000);
  $v2 = $v % 10000;
  echo $v1 . " " . $v2 . "\n";
  ++$a2->value;
}
echo "--------\n";
var_dump($a1, $a2->value);
echo "========\n";
for ($x->next(); $x->valid(); $x->next()) {
  $v = $x->current();
  $v1 = (int)($v / 10000);
  $v2 = $v % 10000;
  echo $v1 . " " . $v2 . "\n";
  ++$a2->value;
}
echo "--------\n";
var_dump($a1, $a2->value);
}
