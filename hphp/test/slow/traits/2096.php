<?php
trait t1 {
 static public $x = INIT1;
 }
class c {
  use t1;
 }


<<__EntryPoint>>
function main_2096() {
define('INIT1', 123);
var_dump(c::$x++);
var_dump(t1::$x++);
var_dump(c::$x++);
var_dump(t1::$x++);
}
