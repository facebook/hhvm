<?php


<<__EntryPoint>>
function main_1148() {
  $arr = array('a' => 'ok');
  extract(&$arr, EXTR_PREFIX_ALL, 'p');
  var_dump($p_a);
}
