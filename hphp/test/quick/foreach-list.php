<?php

function test_simple() {
  $arr = array(
    array(1,2),
    null,
    array(3,4)
  );
  foreach($arr as list($a, $b)) {
    var_dump($a, $b);
  }
}

function test_nested() {
  $arr = array(
    array(1, array(2,3), 4),
    array(5, array(6,7), 8),
    null,
    array(9, array(10, 11), 12)
  );

  foreach ($arr as list($a, list($b, $c), $d)) {
    var_dump($d, $c, $b, $a);
  }
}

function test_single() {
  $arr = array(
    array(1), array(2)
  );
  foreach($arr as list($a)) {
    var_dump($a);
  }
}

function gen() {
  yield array(1,2) => 3;
  yield array(4,5) => 6;
}
function test_key() {
  foreach (gen() as list($a, $b) => $c) {
    var_dump($c, $a, $b);
  }
}

function gen2() {
  yield array(1,array(2,3),4) => array(array(1,2),array(3,4));
  yield array(1,null,2) => array(null, array(1,2));
  yield null => null;
  yield array(1,array(2,3,4),5) => array(array(1,2),array(3,4),array(5,6));
}
function test_complex() {
  foreach (gen2() as
           list($a, list($b, $c), $d) =>
           list(list($e, $f), list($g, $h))) {
    var_dump($b, $a, $d, $c, $f, $e, $h, $g);
  }
}

test_simple();
test_nested();
test_single();
test_key();
test_complex();
