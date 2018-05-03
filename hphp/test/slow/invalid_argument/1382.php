<?php

function bar($a) {
 return $a;
 }
function baz($a) {
 return $a;
 }
function foo($x) {
  return call_user_func('baz', call_user_func('bar', $x));
}
