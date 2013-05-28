<?
define('INIT1', 1000);
trait t {
 static public $x = INIT1;
 }
class c {
 use t;
 }
class d {
 use t;
 }
class e extends d {
}
var_dump(t::$x++);
var_dump(c::$x++);
var_dump(d::$x++);
var_dump(e::$x++);
var_dump(t::$x++);
var_dump(c::$x++);
var_dump(d::$x++);
var_dump(e::$x++);
