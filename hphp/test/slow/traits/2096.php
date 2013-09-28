<?php

define('INIT1', 123);
trait t1 {
 static public $x = INIT1;
 }
class c {
  use t1;
 }
var_dump(c::$x++);
var_dump(t1::$x++);
var_dump(c::$x++);
var_dump(t1::$x++);
