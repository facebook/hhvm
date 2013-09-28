<?php

define('INIT1', "1");
define('INIT2', "2");
trait t1 {
 static public $x = INIT1;
 }
trait t2 {
 static public $y = INIT2;
 }
trait t {
 use t1, t2;
 }
class c {
 use t;
 }
var_dump(c::$x);
var_dump(c::$y);
