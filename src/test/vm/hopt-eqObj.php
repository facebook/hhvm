<?php

class X {}

function foo($x, $y) {
  return $x == $y;
}
$x = new X;
foo($x, $x);


$x = new X;
$y = new stdClass;

function baz($r, $s) {
  switch ($r) {
  case $s: echo 'arg '; break;
  default: echo 'def ';
  }
}
baz($x, $y);

function bal($r, $s) {
  if ($r == $s) {
    echo 'arg ';
  } else {
    echo 'def ';
  }
}
bal($x, $y);

printf("\n");

