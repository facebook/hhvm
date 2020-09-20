<?hh
class State { static $x = 1; }
function foo() {
  State::$x += 10;
  yield State::$x;
  State::$x += 100;
  yield State::$x;
  State::$x += 1000;
  yield State::$x;
}

<<__EntryPoint>>
function main_g8() {
$x = foo();
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
