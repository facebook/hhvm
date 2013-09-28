<?php

function foo($m, $n) {
  $offset_change = 10;
  $offset_change -= strlen($m) - strlen($n);
  var_dump($offset_change);
}
foo('abc', 'efg');
