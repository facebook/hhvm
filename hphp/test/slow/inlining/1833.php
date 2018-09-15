<?php

function g($a) {
 function t(){
}
;
 return $a ? array(1,2,3) : 'foo';
 }
function f($a) {
 return g($a);
 }
function test($a) {
  return reset((f($a)));
  }
function &h(&$a) {
 return $a['foo'];
 }
function i($a) {
 $x = &h($a);
 $x = 'hello';
 return $a;
 }

<<__EntryPoint>>
function main_1833() {
var_dump(test(1));
var_dump(i(false));
}
