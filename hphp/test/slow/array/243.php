<?php

class X {
 function __destruct() {
 var_dump('two');
 }
 }
function test($a) {
  $x = array(new X);
  $a['foo'] = $x;
  var_dump('one');
  var_dump($x);
}

<<__EntryPoint>>
function main_243() {
test(1);
var_dump('three');
}
