<?php

function foo($v) {
  return (array)$v;
}

var_dump(foo(null));
var_dump(foo(true));
var_dump(foo(1));
var_dump(foo(1.1));
var_dump(foo("abc"));
var_dump(foo(array(123)));
class C { public $foo = "bar"; }
var_dump(foo(new C));

function bar($i) {
  $v1 = "undefined";
  $v2 = "undefined";
  $v3 = "undefined";
  $v4 = "undefined";
  $v5 = "undefined";
  $v6 = "undefined";
  $v7 = "undefined";
  $v8 = "undefined";
  $v9 = "undefined";
  $v10 = "undefined";
  $v11 = "undefined";
  if ($i >= 1) {
    $v1 = null;
    $v2 = false;
    $v3 = true;
    $v4 = 0;
    $v5 = 1;
    $v6 = 0.0;
    $v7 = 1.1;
    $v8 = "abc";
    $v9 = new C();
    $v10 = array();
    $v11 = array(123);
  }
  var_dump((array)$v1);
  var_dump((array)$v2);
  var_dump((array)$v3);
  var_dump((array)$v4);
  var_dump((array)$v5);
  var_dump((array)$v6);
  var_dump((array)$v7);
  var_dump((array)$v8);
  var_dump((array)$v9);
  var_dump((array)$v10);
  var_dump((array)$v11);
}

bar(1);

