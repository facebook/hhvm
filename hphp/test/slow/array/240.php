<?php

class A {
  public function __call($method, $args) {
    foreach ($args as $a) {
 var_dump($a);
 }
    var_dump(array_pop($args));
    if (isset($args[1])) {
 var_dump($args[1]);
 }
    reset($args);
    if (key($args) === 0) {
       $args = array(5);
    }
    if (current($args) === 0) {
       $args = array(5);
    }
    if (next($args) === 0) {
       $args = array(5);
    }
    var_dump($args['1']);
    var_dump($args['hi']);
    $args = $args + array(2 => 0, 3 => true, 4 => true);
     var_dump($args);
  }
}
$obj = new A;
$obj->foo(1, 2, 3);
