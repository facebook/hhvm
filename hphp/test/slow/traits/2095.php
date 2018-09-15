<?php
trait t1 {
 static public $x = INIT1;
 }
class c {
 use t1;
 }


<<__EntryPoint>>
function main_2095() {
define('INIT1', 123);
var_dump(c::$x);
}
