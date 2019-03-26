<?php


<<__EntryPoint>>
function main_1089() {
  $ref = 0;
  $a = array('a' => &$ref);
  var_dump($a);
  $b = $a;
  var_dump($a,$b);
  $b['a'] = 1;
  var_dump($a,$b);
  unset($ref);
  $ref = 0;
  $a = array(&$ref);
  var_dump($a);
  $b = $a;
  var_dump($a,$b);
  $b[0] = 1;
  var_dump($a,$b);
}
