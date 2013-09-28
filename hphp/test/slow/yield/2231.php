<?php
function foo() {
  yield 1;
  yield -100 => 2;
  yield 3;
}

function main() {
  foreach (foo() as $k => $v) {
    var_dump($k, $v);
  }
}
main();
