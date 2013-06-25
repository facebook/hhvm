<?php

$arr = new ArrayIterator(array('a','b','c'));

function check_seek($lim, $seek) {
  try {
    $lim->seek($seek);
  } catch (OutOfBoundsException $e) {
    var_dump($e->getMessage());
  }
}

$lim1 = new LimitIterator($arr, 0, 2);
check_seek($lim1, 0);
check_seek($lim1, 1);
check_seek($lim1, 2);

$lim2 = new LimitIterator($arr, 1);
check_seek($lim2, 0);
check_seek($lim2, 1);
check_seek($lim2, 2);
