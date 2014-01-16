<?php

error_reporting(E_ALL);

class TFoo {
	public $c;
	function TFoo($c) {
		$this->c = $c;
	}
	function inc() {
		$this->c++;
	}
}

session_id("abtest");
session_start();

$_SESSION["o1"] = new TFoo(42);
$_SESSION["o2"] =& $_SESSION["o1"];

session_write_close();

unset($_SESSION["o1"]);
unset($_SESSION["o2"]);

session_start();

var_dump($_SESSION);

$_SESSION["o1"]->inc();
$_SESSION["o2"]->inc();

var_dump($_SESSION);

session_destroy();
?>