<?php

class X {
 function foo($a) {
 echo 'In foo:';
 var_dump($a);
 }
 }
function y($y) {
 echo 'In y:';
 var_dump($y);
 }
function test($x, $y) {
  $x->foo($x = null);
  $y($y = null);
}
test(new X, 'y');
