<?php

function test($a, $f) {
  $a[0] = $f;
  $a[$f] = $f;
  $a[1] = 1;
  $a['foo'] = 'foo';
  return $a;
}

var_dump(test("", "f".isset($g)?"x":""));
