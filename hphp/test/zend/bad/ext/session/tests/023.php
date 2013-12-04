<?php
error_reporting(E_ALL);

class foo {
	public $bar = "ok";
	function method() { $this->yes++; }
}

session_id("abtest");
session_start();
session_decode('baz|O:3:"foo":2:{s:3:"bar";s:2:"ok";s:3:"yes";i:1;}arr|a:1:{i:3;O:3:"foo":2:{s:3:"bar";s:2:"ok";s:3:"yes";i:1;}}');
$baz = $_SESSION['baz'];
$arr = $_SESSION['arr'];

$baz->method();
$arr[3]->method();

var_dump($baz);
var_dump($arr);
session_destroy();