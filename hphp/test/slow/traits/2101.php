<?php
trait t2 {
 static public $y = INIT2;
 }
trait t1 {
 use t2;
 static public $x = INIT1;
 }
class c {
 use t1;
 }


<<__EntryPoint>>
function main_2101() {
define('INIT1', 123);
define('INIT2', 456);
var_dump(c::$x);
var_dump(c::$y);
}
