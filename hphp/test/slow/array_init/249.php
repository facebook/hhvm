<?php

$v = 1;
function foo($a) {
  $arr = array($a, $a++, $a);
  var_dump($arr);
}
foo($v);
