<?php
trait t1 {
 public $x = INIT1;
 }
trait t2 {
 public $y = INIT2;
 }
trait t {
 use t1, t2;
 }
class c {
 use t;
 }


<<__EntryPoint>>
function main_2090() {
define('INIT1', "1");
define('INIT2', "2");
$obj = new c;
var_dump($obj->x);
var_dump($obj->y);
}
