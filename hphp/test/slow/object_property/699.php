<?php
class d {
 static public $x = INIT1;
 }
class e extends d {
}


<<__EntryPoint>>
function main_699() {
define('INIT1', 1000);
var_dump(d::$x++);
var_dump(e::$x++);
}
