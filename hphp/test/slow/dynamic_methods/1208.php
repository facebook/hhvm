<?php

class A {
 function _test() {
 print 'ok';
}
 function __call($name, $args) {
 $name = '_'.$name;
 $this->$name();
}
 }
$obj = new A();
 $obj->test();
