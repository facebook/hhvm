<?php
trait t2 {
 public $y = INIT2;
 }
trait t1 {
 use t2;
 public $x = INIT1;
 }
class c {
 use t1;
 }


<<__EntryPoint>>
function main_2094() {
define('INIT1', 123);
define('INIT2', 456);
$obj = new c;
var_dump($obj->x);
var_dump($obj->y);
}
