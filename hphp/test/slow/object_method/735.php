<?php

function foo() {
 echo 'Caught';
 exit;
 }
set_error_handler('foo', E_ALL);
class X {
 function foo() {
 var_dump($this);
 }
 }
class Y {
  function bar(X $a) {
 $a->foo();
 }
}
function test($y,$z) {
  $y->$z($y);
}
test(new Y, 'bar');
