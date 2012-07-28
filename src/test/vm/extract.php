<?php

function f1() {
  $x = 0;
  $a = array("x" => array(1, 2, "foo"), "y" => &$x, "z" => &$y);
  extract($a);
  var_dump($x);
  var_dump($y);
  $c = "z";
  var_dump($$c);
}

function f2() {
  $x = 0;
  $a = array("x" => array(1, 2, "foo"), "y" => &$x, "z" => &$y);
  extract($a, EXTR_SKIP);
  var_dump($x);
  var_dump($y);
  var_dump($z);
}

function f3() {
  $x = 0;
  $a = array("x" => array(1, 2, "foo"), "y" => &$x, "z" => &$y);
  extract($a, EXTR_PREFIX_SAME, "f3");
  var_dump($f3_x);
  var_dump($f3_y);
  var_dump($z);
}

function f4() {
  $x = 0;
  $a = array("x" => array(1, 2, "foo"), "y" => &$x, "z" => &$y);
  extract($a, EXTR_PREFIX_ALL, "f4");
  var_dump($f4_x);
  var_dump($f4_y);
  $c = "f4_z";
  var_dump($$c);
}

function f5() {
  $x = 0;
  $a = array("x" => array(1, 2, "foo"), "y" => &$x, "z" => &$y);
  extract($a, EXTR_PREFIX_INVALID, "f5");
  var_dump($x);
  var_dump($y);
  var_dump($z);
}

function f6() {
  $x = 0;
  $a = array("x" => array(1, 2, "foo"), "y" => &$x, "z" => &$y);
  extract($a, EXTR_IF_EXISTS);
  var_dump($x);
  var_dump($y);
}

function f7() {
  $x = 0;
  $a = array("x" => array(1, 2, "foo"), "y" => &$x, "z" => &$y);
  extract($a, EXTR_PREFIX_IF_EXISTS, "f7");
  var_dump($f7_x);
  var_dump($f7_y);
}

function f8() {
  $x = 0;
  $a = array("x" => array(1, 2, "foo"), "y" => &$x, "z" => &$y);
  extract($a, EXTR_REFS);
  var_dump($x);
  var_dump($y);
  $c = "z";
  var_dump($$c);
}

function main() {
  f1();
  f2();
  f3();
  f4();
  f5();
  f6();
  f7();
  f8();
}
main();

