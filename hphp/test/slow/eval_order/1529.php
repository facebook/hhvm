<?php

function test($x) {
  $a = array($a => $x[$a = 'foo']);
  return $a;
}
var_dump(test(array('foo' => 5)));
