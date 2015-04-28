<?php

function x($x) {
  $z = new stdclass;
  $k = new stdclass;
  $p = new stdclass;
  $l = new stdclass;
  hash($x, $x);
  return $x;
}

x("asd".mt_rand());
x("asd".mt_rand());
x("asd".mt_rand());
x("asd".mt_rand());
x("asd".mt_rand());
