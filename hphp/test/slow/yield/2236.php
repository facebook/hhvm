<?php

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
main();
