<?php

function main() {
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
}
main();
main();

function strswitch() {
  $a = 'luke';
  switch ($a) {
    case 'leia':
      echo "nope\n";
      break;
    case 'luke':
      echo "yep\n";
      break;
    default:
      echo "whoops\n";
      break;
  }

  $x = '123';
  switch ($x) {
    case '123.0':
      echo "right\n";
      break;

    case '123':
      echo "wrong\n";
      break;
  }

  switch($x) {
    case 'wheeeeeee':
      return -1;
    case 'whooooooo':
      return -2;
    default:
      echo "cool\n";
  }

  return true;
}
var_dump(strswitch());
var_dump(strswitch());
