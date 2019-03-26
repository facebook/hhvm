<?php


<<__EntryPoint>>
function main() {
  $tmp = 456;
  $a = [123, &$tmp];
  var_dump($a);
  $b = array_combine($a, $a);
  var_dump($a);
  var_dump($b);
}
