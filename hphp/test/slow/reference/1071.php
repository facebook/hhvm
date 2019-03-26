<?php


<<__EntryPoint>>
function main_1071() {
  $b = 1;
  $a = array('t' => &$b);
  $b = 2;
  var_dump($a);
}
