<?php

define('INIT1', 123);
trait t1 {
 public $x = INIT1;
 }
class c {
 use t1;
 }
$obj = new c;
var_dump($obj->x);
