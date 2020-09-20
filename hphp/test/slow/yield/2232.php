<?hh

function foo() {
  yield 1;
  yield 100 => 2;
  yield 10 => 3;
  yield 4;
}

function main() {
  foreach(foo() as $k => $v) {
    var_dump($k, $v);
  }
}

<<__EntryPoint>>
function main_2232() {
main();
}
