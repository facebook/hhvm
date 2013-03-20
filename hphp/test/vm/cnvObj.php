<?php

function foo($v) {
  return (object)$v;
}

var_dump(foo(null));
var_dump(foo(false));
var_dump(foo(true));
var_dump(foo(1));
var_dump(foo(1.1));
var_dump(foo("abc"));
var_dump(foo(array(123)));
class C { }
var_dump(foo(new C));

function bar($v) {
  $nonStaticStr = (string)$v;
  return (object)$nonStaticStr;
}
var_dump(bar(1));

function candy($i) {
  $v1 = "undefined";
  if ($i >= 1) {
    $v1 = null;
  }
  var_dump((object)$v1);
}

candy(1);