<?php

<<__EntryPoint>>
function main_1813() {
  $val = 1;
  $a = array(&$val, &$val);
  unset($val);
  $a[0] = 2;
  print_r($a);

  apc_store('table', $a);
  $b = apc_fetch('table', &$b);
  print_r($b);
  $b[0] = 3;
  print_r($b);

  $a = array('xyz' => 'tuv', &$val, &$val);
  unset($val);
  $a[0] = 2;
  print_r($a);

  apc_store('table', $a);
  $b = apc_fetch('table', &$b);
  print_r($b);
  $b[0] = 3;
  print_r($b);
}
