<?php

function f() {
  $a = array();
  $a[] = 1;
  try { array_push(&$a); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  var_dump($a);
}

<<__EntryPoint>>
function main_1713() {
f();
}
