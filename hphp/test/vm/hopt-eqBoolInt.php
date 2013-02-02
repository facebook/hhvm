<?php

function cmpCTrue($x) {
  print "----------\n";
  print "x = ";
  var_dump($x);
  print "(x == true) = ";
  var_dump($x == true);
  print "(true == x) = ";
  var_dump(false == $x);
}

function cmpCFalse($x) {
  print "----------\n";
  print "x = ";
  var_dump($x);
  print "(x == false) = ";
  var_dump($x == false);
  print "(false == x) = ";
  var_dump(false == $x);
}

function cmpC0($x) {
  print "----------\n";
  print "x = ";
  var_dump($x);
  print "(x == 0) = ";
  var_dump($x == 0);
  print "(0 == x) = ";
  var_dump(0 == $x);
}

function cmpC1($x) {
  print "----------\n";
  print "x = ";
  var_dump($x);
  print "(x == 1) = ";
  var_dump($x == 1);
  print "(1 == x) = ";
  var_dump(1 == $x);
}

function cmpC2($x) {
  print "----------\n";
  print "x = ";
  var_dump($x);
  print "(x == 2) = ";
  var_dump($x == 2);
  print "(2 == x) = ";
  var_dump(2 == $x);
}

function cmpC3($x) {
  print "----------\n";
  print "x = ";
  var_dump($x);
  print "(x == 3) = ";
  var_dump($x == 3);
  print "(3 == x) = ";
  var_dump(3 == $x);
}

function cmp3($x, $y) {
  print "----------\n";
  print "x = ";
  var_dump($x);
  print "y = ";
  var_dump($y);
  print "(x == y) = ";
  var_dump($x == $y);
  print "(x != y) = ";
  var_dump($x != $y);
}

function cmp2($x, $y) {
  cmp3($x, $y);
  cmp3($y, $x);
}

function cmp1($x) {
  cmp2($x, true);
  cmp2($x, false);
  cmp2($x, 0);
  cmp2($x, 1);
  cmp2($x, 2);
  cmp2($x, 3);
  cmpCTrue($x);
  cmpCFalse($x);
  cmpC0($x);
  cmpC1($x);
  cmpC2($x);
  cmpC3($x);
}

function cmp() {
  cmp1(true);
  cmp1(false);
  cmp1(0);
  cmp1(1);
  cmp1(2);
  cmp1(1234567);
}

cmp();

