<?php

class B {
}
class A extends B {
  function __call($name,$args) {
 echo 'A::$name
';
 }
}

<<__EntryPoint>>
function main_1472() {
;
if (0) {
  include '1472.inc';
}
;
$a = new A;
call_user_func_array(array($a, 'foo'), array());
}
