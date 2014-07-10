<?php
function main() {
  $a = array();
  $x = array_shift($a);
  var_dump($x);
  unset($a, $x);
  $a = array();
  $x = array_pop($a);
  var_dump($x);
  unset($a, $x);
  $a = array();
  $x = next($a);
  var_dump($x);
  unset($a, $x);
  $a = array();
  $x = prev($a);
  var_dump($x);
  unset($a, $x);
}
main();
