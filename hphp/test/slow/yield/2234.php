<?hh

function foo() {
  yield PHP_INT_MAX => 1;
  yield 2;
}

function main() {
  foreach (foo() as $k => $v) {
    var_dump($k, $v);
  }
}

<<__EntryPoint>>
function main_2234() {
main();
}
