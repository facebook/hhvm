<?hh

class Ref {
  function __construct(public $value)[] {}
}
function f($a) :AsyncGenerator<mixed,mixed,void>{
  ++$a->value;
  yield $a->value;
  ++$a->value;
  yield $a->value;
}

<<__EntryPoint>>
function main_g11() :mixed{
$a = new Ref(3);
$x = f($a);
unset($a);
$y1 = clone $x;
$y2 = clone $x;
foreach ($y1 as $v) {
  echo $v . "\n";
}
echo "========\n";
foreach ($y2 as $v) {
  echo $v . "\n";
}
echo "========\n";
foreach ($x as $v) {
  echo $v . "\n";
}
}
