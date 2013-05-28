<?php

define('INIT1', 123);
trait t2 {
 static public $x = INIT1;
 }
trait t1 {
 use t2;
 }
class c {
 use t1;
 }
var_dump(c::$x);
