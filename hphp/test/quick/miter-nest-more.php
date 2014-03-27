<?php

function blah(&$val) {
  global $x;
  ++$x;
}

function heh() { return 3; }

function foo() {
  $x0 = array(1,2);
  $x1 = array(1);
  $x2 = array(1);
  $x3 = array(1);
  $x4 = array(1);
  $x5 = array(1);
  $x6 = array(1);
  $x7 = array(1);
  $x8 = array(1);
  $x9 = array(1);
  $xA = array(1,2,3,heh());
  $xB = array(heh());
  $xC = array(heh());
  $xD = array(heh());
  $xE = array(heh());
  $xF = array(1,2,3,4,heh());
  $y0 = array(heh());
  $y1 = array(heh());
  $y2 = array(heh());
  $y3 = array(1,2,heh());

  foreach ($x0 as &$v0)
  foreach ($x1 as &$v1)
  foreach ($x2 as &$v2)
  foreach ($x3 as &$v3)
  foreach ($x4 as &$v4)
  foreach ($x5 as &$v5)
  foreach ($x6 as &$v6)
  foreach ($x7 as &$v7)
  foreach ($x8 as &$v8)
  foreach ($x9 as &$v9)
  foreach ($xA as &$vA)
  foreach ($xB as &$vB)
  foreach ($xC as &$vC)
  foreach ($xD as &$vD)
  foreach ($xE as &$vE)
  foreach ($xF as &$vF)
  foreach ($y0 as &$V0)
  foreach ($y1 as &$V1)
  foreach ($y2 as &$V2)
  foreach ($y3 as &$V3) {
    blah($v9);
    blah($V3);
  }
}

foo();
var_dump($x);
