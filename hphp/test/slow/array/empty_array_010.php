<?php

function main() {
  $x = array();
  $x[42] = 2;
  $x[] = 3;
  var_dump($x);

  $x = array();
  $x[42][42] = 2;
  $x[] = 3;
  var_dump($x);

  $x = array();
  $x[PHP_INT_MAX] = 2;
  $x[] = 3;
  var_dump($x);

  $x = array();
  $x[PHP_INT_MAX][42] = 2;
  $x[] = 3;
  var_dump($x);
}

main();
