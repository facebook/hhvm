<?php

function h() {
 class X{
}
;
 }
function f($a, $b, $c) {
 return h();
 }
function g($a, $b, $c) {
  return f($a++, $b++ + $a++, $c);
}
