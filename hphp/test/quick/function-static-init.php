<?php

function f() {
  static $x = 40;
  var_dump($x);
  $x++;
}

f();
f();
f();


function blerg() {
  static $a = array(ICHI, NI, SAN);
  var_dump($a);
  $a []= count($a) + 1;
}

define("ICHI", 1);
define("NI", 2);
define("SAN", 3);
blerg();
blerg();
blerg();

function n() {
  static $x;
  var_dump($x);
}

n();
