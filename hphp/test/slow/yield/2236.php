<?hh

function foo() {
  $a = new stdClass;
  yield $a => 1;
  yield $a => 2;
  yield $a => 3;
}

function main() {
  foreach(foo() as $k => $v) {
    var_dump($k, $v);
  }
}

<<__EntryPoint>>
function main_2236() {
main();
}
