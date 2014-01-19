<?php

static $a = 1, $b = 2;
static $c = 1;
static $d = 1;
static $e = 1;
static $f = 1;
static $g = 1;
static $h = 1;
static $i = 1;
static $i = 2;
if (false) {
  static $a = 2;
  static $b = 3;
  static $c = 2;
  static $g;
  static $i;
  $e = 2;
}
 else {
  static $d = 2;
  static $h;
  $f = 2;
}
echo $a;
echo $b;
echo $c;
echo $d;
echo $e;
echo $f;
echo $g;
echo $h;
echo $i;
function f() {
  static $a = 1, $b = 2;
  static $c = 1;
  static $d = 1;
  static $e = 1;
  static $f = 1;
  static $g = 1;
  static $h = 1;
  static $i = 1;
  static $i = 2;
  if (false) {
    static $a = 2;
    static $b = 3;
    static $c = 2;
    static $g;
    static $i;
    $e = 2;
  }
 else {
    static $d = 2;
    static $h;
    $f = 2;
  }
  echo $a;
  echo $b;
  echo $c;
  echo $d;
  echo $e;
  echo $f;
  echo $g;
  echo $h;
  echo $i;
}
f();
class foo {
  static $a = 1, $b = 2;
  static $c = 1;
  static $d = 1;
  static $e = 1;
  static $f = 1;
  function bar() {
    static $a = 1, $b = 2;
    static $c = 1;
    static $d = 1;
    static $e = 1;
    static $f = 1;
    static $g = 1;
    static $h = 1;
    static $i = 1;
    static $i = 2;
    if (false) {
      static $a = 2;
      static $b = 3;
      static $c = 2;
      static $g;
      static $i;
      $e = 2;
    }
 else {
      static $d = 2;
      static $h;
      $f = 2;
    }
    echo foo::$a;
    echo foo::$b;
    echo foo::$c;
    echo foo::$d;
    echo foo::$e;
    echo foo::$f;
    echo $a;
    echo $b;
    echo $c;
    echo $d;
    echo $e;
    echo $f;
    echo $g;
    echo $h;
    echo $i;
  }
}
echo foo::$a;
echo foo::$b;
$v = new foo;
$v->bar();
