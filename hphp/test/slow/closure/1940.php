<?php

function mkC() {
  return function () {
    static $x = 0;
    return $x++;
  }
;
}
$c0 = mkC();
var_dump($c0());
var_dump($c0());

$c1 = mkC();
var_dump($c1());
var_dump($c1());
