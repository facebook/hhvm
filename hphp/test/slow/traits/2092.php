<?php

define('INIT1', 123);
define('INIT2', 456);
trait t1 {
 public $x = INIT1;
 }
class c {
 use t1;
 public $y = INIT2;
 }
$obj = new c;
var_dump($obj->x);
var_dump($obj->y);
