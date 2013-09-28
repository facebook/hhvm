<?php

function foo() {
  $i = 0;
  foreach (range("a","e") as $letter) {
    yield $letter => ++$i;
  }
}

foreach (foo() as $k => $v) {
  var_dump($k);
  var_dump($v);
}
