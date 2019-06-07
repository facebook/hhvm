<?hh
function foo() {
  yield 1 => 2;
  yield 'a' => 'b';
}
function bar() {
  foreach (foo() as $k => $v) {
    yield $k => $v;
  }
}
function main() {
  foreach (bar() as $k => $v) {
    echo "$k $v\n";
  }
}


<<__EntryPoint>>
function main_yield_key_value_bug() {
main();
}
