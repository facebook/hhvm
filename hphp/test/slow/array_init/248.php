<?php

function foo($p) {
  $a = array('a', 'b', $p);
  $a[] = 'd';
  var_dump($a);
  $a = array(0 => 'a', 1 => 'b', 2 => $p);
  $a[] = 'd';
  var_dump($a);
  $a = array(2 => 'a', 4 => 'b', 6 => $p);
  $a[] = 'd';
  var_dump($a);
  $a = array(-2 => 'a', -4 => 'b', -6 => $p);
  $a[] = 'd';
  var_dump($a);
  $a = array(0 => 'a');
  $a[] = 'b';
  var_dump($a);
}
foo('c');
