<?php

function get() {
 return true;
 }

<<__EntryPoint>>
function main_1939() {
if (get()) {
  function f($x) {
    return function () use ($x) {
 return $x;
 }
;
  }
}
 else {
  function f($x) {
    return function () use ($x) {
 return $x + 1;
 }
;
  }
}
$f = f(32);
var_dump($f());
}
