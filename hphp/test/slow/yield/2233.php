<?php

function foo() {
  yield 1.0 => "hello";
  yield array(1,2,3) => "world";
  yield new stdClass => "foobar";
}

function main() {
  foreach (foo() as $k => $v) {
    var_dump($k, $v);
  }
}
main();
