<?php

define('INIT1', 123);
define('INIT2', 456);
trait t1 {
 static public $x = INIT1;
 }
class c {
 use t1;
 static public $y = INIT2;
 }
var_dump(c::$x);
var_dump(c::$y);
