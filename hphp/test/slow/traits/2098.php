<?php
trait t1 {
}
class c {
 use t1;
 static public $x = INIT1;
 }


<<__EntryPoint>>
function main_2098() {
define('INIT1', 123);
var_dump(c::$x);
}
