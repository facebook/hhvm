<?php

function foo() {
  $u = "abc";
  $v = "\0";
  $w = "def\n";
  $x = $u . $v . $w;
  echo $x;
  echo "abc" . "\0" . "def\n";
}
foo();
$u = "abc";
$v = "\0";
$w = "def\n";
$x = $u . $v . $w;
echo $x;
echo "abc" . "\0" . "def\n";
echo "ab\0c\n";
