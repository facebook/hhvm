<?php

function test($a) {
  list($a[0], $a[1], $a) = $a;
  var_dump($a);
}
test(array('abc', 'cde', 'fgh'));
