<?php

class A {
   public $a = array();
   function __set($name, $value) {
 $this->a[$name] = $value.'set';
}
   function __get($name) {
 return $this->a[$name].'get';
}
 }
 $obj = new A();
 $obj->test = 'test';
 var_dump($obj->test);
