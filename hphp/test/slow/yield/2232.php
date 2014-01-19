<?php

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
main();
