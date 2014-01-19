<?php

class A {
 public $test = 'ok';
}
 $obj = new A();
 var_dump($obj);
var_dump((bool)$obj);
var_dump((int)$obj);
var_dump((array)$obj);
var_dump((object)$obj);
