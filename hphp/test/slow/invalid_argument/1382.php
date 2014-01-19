<?php

function bar($a) {
 return $a;
 }
function baz($a) {
 return $a;
 }
function foo($x) {
  return fb_call_user_func_safe_return('baz',         fb_call_user_func_safe_return('bar', $x));
}
