<?php
trait t1 {
 public $x = INIT1;
 }
class c {
 use t1;
 public $y = INIT2;
 }


<<__EntryPoint>>
function main_2092() {
define('INIT1', 123);
define('INIT2', 456);
$obj = new c;
var_dump($obj->x);
var_dump($obj->y);
}
