<?php


<<__EntryPoint>>
function main_1147() {
  $a = 1;
  $arr = array('a' => 'ok');
  extract(&$arr, EXTR_PREFIX_SAME, 'p');
  var_dump($p_a);
}
