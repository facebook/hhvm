<?php
trait t2 {
 public $x = INIT1;
 }
trait t1 {
 use t2;
 }
class c {
 use t1;
 }


<<__EntryPoint>>
function main_2093() {
define('INIT1', 123);
$obj = new c;
var_dump($obj->x);
}
