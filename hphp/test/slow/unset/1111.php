<?php

class A {
 public $arr;
}
 $obj = new A;
 $obj->arr[] = 'test';
var_dump($obj->arr);
 unset($obj->arr);
 var_dump($obj->arr);
