<?php

function a() {
  $array1 = array(
    "color" => "red",
    2,
    4
  );
  $array2 = array(
    "a",
    "b",
    "color" => "green",
    "shape" => "trapezoid",
    4
  );
  $result = array_merge($array1, array($array2));
  var_dump($result);
}

function b() {
  $array1 = array();
  $array2 = array(1 => "data");
  $result = array_merge($array1, array($array2));
  var_dump($result);
}

function c() {
  $array1 = array();
  $array2 = array(1 => "data");
  $result = $array1 + $array2;
  var_dump($result);
}

function d() {
  $beginning = "foo";
  $end = array(1 => "bar");
  $result = array_merge((array)$beginning, array($end));
  var_dump($result);
}

function e() {
  $v = 1;
  $a = array("one" => 1);
  $b = array("two" => &$v);
  $r = array_merge($a, array($b));
  $v = 2;
  var_dump($r);
}

function f() {
  $id = 100000000000022;
  $a = array($id => 1);
  $b = array($id => 2);
  $r = array_merge($a, array($b));
  var_dump($r);
}

function g() {
  $a = array(1 => 50, 5 => 60);
  $b = null;
  var_dump(array_merge($a, array($b)));
}

a();
b();
c();
d();
e();
f();
