<?php

class A {
   public $a = array();
   function __set($name, $value) {
 $this->a[$name] = $value;
}
   function __get($name) {
 return $this->a[$name];
}
 }
 $obj = new A();
 $obj->test = 'test';
 var_dump($obj->test);
