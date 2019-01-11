<?php

class c {
 function f(&$a, $b) {
 $a = $b;
 }
 }
function setNullVariantHelper($f, $value) {
  $a = array();
  $obj = new c;
  $a[$obj] = 1;
  $obj->$f(&$a[$obj], $value);
  var_dump($a[$obj] = 1);
}
function setNullVariant($value) {
  setNullVariantHelper('f', $value);
}

<<__EntryPoint>>
function main_513() {
setNullVariant('Surprise!');
$b = null;
var_dump($b[1]);
}
