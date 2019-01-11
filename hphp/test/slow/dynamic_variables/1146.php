<?php


<<__EntryPoint>>
function main_1146() {
  $a = 1;
  $arr = array('a' => 'ok');
  extract(&$arr, EXTR_SKIP);
  var_dump($a);
}
