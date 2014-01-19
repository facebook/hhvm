<?php

function foo($x = null) {
  if ($x) $x = 'foo';
  var_dump($x);
  yield 1;
  }
foreach(foo() as $x) {
}
