<?hh
class State { public static $x = 1; }
function foo() :AsyncGenerator<mixed,mixed,void>{
  State::$x += 10;
  yield State::$x;
  State::$x += 100;
  yield State::$x;
  State::$x += 1000;
  yield State::$x;
}

<<__EntryPoint>>
function main_g7() :mixed{
$x = foo();
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
