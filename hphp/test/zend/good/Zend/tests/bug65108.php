<?php

class E {
   private function f() {}
   function __call($name, $args) {}
}
$isCallable = is_callable(array('E', 'f'));
var_dump($isCallable);
