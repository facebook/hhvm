<?php

$gen = (new class() {
	function a() {
		yield "okey";
	}
})->a();

var_dump($gen->current());
?>
