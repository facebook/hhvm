<?php

function f() {
  $arr = array();
  sort(&$arr[0]);
  var_dump($arr);
  unset($arr);
  $arr = array();
  try { sort(&$arr[0],0,0,0,0,0,0,0,0); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  var_dump($arr);
}

<<__EntryPoint>>
function main_1716() {
f();
}
