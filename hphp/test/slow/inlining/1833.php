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
var_dump(test(1));
function &h(&$a) {
 return $a['foo'];
 }
function i($a) {
 $x = &h($a);
 $x = 'hello';
 return $a;
 }
var_dump(i(false));
