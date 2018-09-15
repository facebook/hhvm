<?php

function test($a, $f) {
  $a[0] = $f;
  $a[$f] = $f;
  $a[1] = 1;
  $a['foo'] = 'foo';
  return $a;
}


<<__EntryPoint>>
function main_promote_string() {
var_dump(test("", "f".isset($g)?"x":""));
}
