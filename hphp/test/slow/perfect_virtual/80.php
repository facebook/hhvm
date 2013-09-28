<?php

class X {
  function foo($a) {
 return $a;
 }
}
class Y extends X {
  function &foo($a) {
 return $a;
 }
}
function test(X $x) {
  $y = $x->foo(5);
  return ++$y;
}
test(new Y);
