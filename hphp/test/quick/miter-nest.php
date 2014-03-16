<?php

function blah(&$val) {
  global $x;
  ++$x;
}

function foo() {
  $x0 = array(1,2,3);
  $x1 = array(1,2,3);
  $x2 = array(1,2,3);
  $x3 = array(1,2,3);
  $x4 = array(1,2,3);
  $x5 = array(1,2,3);
  $x6 = array(1,2,3);
  $x7 = array(1,2,3);
  $x8 = array(1,2,3);
  $x9 = array(1,2,3);

  foreach ($x0 as &$v0)
  foreach ($x1 as &$v1)
  foreach ($x2 as &$v2)
  foreach ($x3 as &$v3)
  foreach ($x4 as &$v4)
  foreach ($x5 as &$v5)
  foreach ($x6 as &$v6)
  foreach ($x7 as &$v7)
  foreach ($x8 as &$v8)
  foreach ($x9 as &$v9)   blah($v9);
}

foo();
var_dump($x);
