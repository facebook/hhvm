<?php

function foo() {
  yield 1;
  yield 2;
  yield "abc" => 3;
  yield 4;
  yield 10 => 5;
  yield 6;
}

foreach(foo() as $k => $v) {
  var_dump($k);
  var_dump($v);
}
