<?php

class B {
}
;
if (0) {
  class B {
}
}
class A extends B {
  function __call($name,$args) {
 echo 'A::$name
';
 }
}
;
$a = new A;
call_user_func_array(array($a, 'foo'), array());
