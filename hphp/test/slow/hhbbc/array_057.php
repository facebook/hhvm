<?php

function unknown($x) {
  return $GLOBALS['asd'];
}

function foo($ids) {
  $x = array();
  foreach ($ids as $id) {
    $target = unknown($id);
    if ($target !== null) {
      $x[$target][] = $id;
    }
  }
  return $x;
}

$asd = '2'.mt_rand();
function main() {
  $x = foo(array(1,2,3));
  foreach ($x as $k => $v) {
    var_dump($v);
  }
}

main();
