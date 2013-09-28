<?php

function foo() {
  yield "abc" => "def";
}

$x = foo();
$y = clone $x;
foreach($x as $k => $v) {
  var_dump($k, $v);
}
foreach($y as $k => $v) {
  var_dump($k, $v);
}
