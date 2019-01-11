<?php


<<__EntryPoint>>
function main_1151() {
  $a = null;
  $arr = array('a' => 'ok', 'b' => 'no');
  extract(&$arr, EXTR_PREFIX_IF_EXISTS, 'p');
  var_dump($p_a);
  var_dump($b);
  var_dump($p_b);
}
