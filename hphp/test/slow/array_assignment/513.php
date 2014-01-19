<?php

class c {
 function f(&$a, $b) {
 $a = $b;
 }
 }
function setNullVariantHelper($f, $value) {
  $a = array();
  $obj = new c;
  $obj->$f($a[$obj] = 1, $value);
  var_dump($a[$obj] = 1);
}
function setNullVariant($value) {
  setNullVariantHelper('f', $value);
}
setNullVariant('Surprise!');
$b = null;
var_dump($b[1]);
