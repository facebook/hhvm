<?php

;
class B1 {
}
class C1 {
 function __construct() {
}
 }
class D1 {
 function __destruct() {
 var_dump(__METHOD__);
 }
 }
class D2 extends C1 {
 function __destruct(){
 var_dump(__METHOD__);
 }
 }
class D3 extends D2 {
}
class D4 extends B1 {
 function __destruct(){
 var_dump(__METHOD__);
 }
 }
class D5 extends D4 {
}
class D6 extends D1 {
 function __construct($a) {
 if ($a) f();
 }
 }
function f() {
 throw new Exception('throw');
 }
function foo($a,$b) {
  try {
    $x = new D6($b?f():$a);
  }
 catch (Exception $e) {
    var_dump('caught');
  }
}
function bar($x, $a, $b) {
  try {
    $x = new $x($b?f():$a);
  }
 catch (Exception $e) {
    var_dump('caught');
  }
}
function n($x) {
 return new $x;
 }
function baz($d) {
  $x = new D1;
  $x = new D2;
  $x = new D3;
  $x = new D4;
  $x = new D5;
  $x = new D6;
  $x = n($d.'1');
  $x = n($d.'2');
  $x = n($d.'3');
  $x = n($d.'4');
  $x = n($d.'5');
  $x = n($d.'6');
  $x = n('B1');
}
foo(false,false);
foo(false,true);
foo(true,true);
foo(true,false);
bar('D6',false,false);
bar('D6',false,true);
bar('D6',true,false);
bar('D6',true,true);
baz('D');
