<?php
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
main();

