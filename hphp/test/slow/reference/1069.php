<?php


<<__EntryPoint>>
function main_1069() {
  $b = 10;
  $a = array(&$b);
  $b = 20;
  var_dump($a);
}
