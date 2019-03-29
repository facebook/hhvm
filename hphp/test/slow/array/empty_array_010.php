<?php

function main() {
  $x = array();
  $x[42] = 2;
  $x[] = 3;
  var_dump($x);

  $x = array();
  $x[PHP_INT_MAX] = 2;
  $x[] = 3;
  var_dump($x);
}


<<__EntryPoint>>
function main_empty_array_010() {
main();
}
