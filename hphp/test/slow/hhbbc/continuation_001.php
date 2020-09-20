<?hh

function foo() {
  yield 1;
  yield 2;
  yield 3;
}

function bar() {
  $x = foo();
  foreach ($x as $k) { echo $k; echo "\n"; }
}


<<__EntryPoint>>
function main_continuation_001() {
bar();
}
