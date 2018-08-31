<?php
trait t1 {
 static public $x = INIT1;
 }
class c {
 use t1;
 static public $y = INIT2;
 }


<<__EntryPoint>>
function main_2099() {
define('INIT1', 123);
define('INIT2', 456);
var_dump(c::$x);
var_dump(c::$y);
}
