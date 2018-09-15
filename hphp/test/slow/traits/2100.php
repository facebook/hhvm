<?php
trait t2 {
 static public $x = INIT1;
 }
trait t1 {
 use t2;
 }
class c {
 use t1;
 }


<<__EntryPoint>>
function main_2100() {
define('INIT1', 123);
var_dump(c::$x);
}
