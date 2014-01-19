<?php

function test($a){
  if ($a > 0) {
    $sql = 'foo';
  }
 else {
    $sql = 'bar';
  }
  $sql .= ' baz';
  return $sql;
}
echo test(1),test(-1),"\n";
