<?php
class Cls {
	function __call($name, $arg) {
	}
	static function __callStatic($name, $arg) {
	}
}

$cls = new Cls;
$cls->{0}();
$cls->{1.0}();
$cls->{true}();
$cls->{false}();
$cls->{null}();

Cls::{0}();
Cls::{1.0}();
Cls::{true}();
Cls::{false}();
Cls::{null}();

?>