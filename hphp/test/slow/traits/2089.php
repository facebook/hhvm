<?php
trait t1 {
 public $x = INIT1;
 }
class c {
 use t1;
 }


<<__EntryPoint>>
function main_2089() {
define('INIT1', 123);
$obj = new c;
var_dump($obj->x);
}
