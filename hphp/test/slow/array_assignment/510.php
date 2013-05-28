<?php

function foo() {
  $p = 1;
  $q = 2;
  $r = 3;
  $s = 4;
  $a = array('1'=>$p, '2'=>&$q);
  $b = array('3'=>$r, '4'=>&$s);
  var_dump($a);
  $a += $b;
  var_dump($a);
  var_dump($b);
}
foo();
