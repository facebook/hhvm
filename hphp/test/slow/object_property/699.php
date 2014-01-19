<?php

define('INIT1', 1000);
class d {
 static public $x = INIT1;
 }
class e extends d {
}
var_dump(d::$x++);
var_dump(e::$x++);
