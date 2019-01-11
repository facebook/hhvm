<?php


<<__EntryPoint>>
function main_1150() {
  $a = null;
  $arr = array('a' => 'ok');
  extract(&$arr, EXTR_IF_EXISTS);
  var_dump($a);
}
