<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function foo(num $x):string {
  return "a";
}

function testit():string {
  $f = $x ==> foo($x);
  $z = $f(3);
  return $f(3.4);
}
