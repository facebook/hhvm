<?php

$a = 2;

switch ($a) {
  case ++$a:
    echo "correct!\n";
    break;
  case 2:
    echo "nope: two\n";
    break;
  default:
    echo "nope: default\n";
    break;
}


$a = array(2);

switch ($a[0]) {
  case ++$a[0]:
    echo "nope: pre-inc\n";
    break;
  case 2:
    echo "correct!\n";
    break;
  default:
    echo "nope: default\n";
    break;
}

$a = array(10);
$ten = 10;
switch ($a[0]) {
  case $ten:
    echo "correct!\n";
    break;
  default:
    echo "nope\n";
    break;
}
