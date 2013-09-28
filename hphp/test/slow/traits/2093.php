<?php

define('INIT1', 123);
trait t2 {
 public $x = INIT1;
 }
trait t1 {
 use t2;
 }
class c {
 use t1;
 }
$obj = new c;
var_dump($obj->x);
