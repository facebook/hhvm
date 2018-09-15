<?php
trait t1 {
}
class c {
 use t1;
 public $x = INIT1;
 }


<<__EntryPoint>>
function main_2091() {
define('INIT1', 123);
$obj = new c;
var_dump($obj->x);
}
