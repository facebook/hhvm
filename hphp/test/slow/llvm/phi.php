<?php

function test_InitPackedArrayLoop($x) {
  $arr = array($x, $x, $x, $x, $x, $x, $x, $x, $x);
  return $arr;
}
var_dump(test_InitPackedArrayLoop(42));
