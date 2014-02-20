<?php
error_reporting(E_ALL);

class foo {
	public $bar = "ok";

	function method() { $this->yes = "done"; }
}

$baz = new foo;
$baz->method();

$arr[3] = new foo;
$arr[3]->method();
session_start();
$_SESSION["baz"] = $baz;
$_SESSION["arr"] = $arr;
var_dump(session_encode());
session_destroy();
?>