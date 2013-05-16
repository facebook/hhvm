<?php

function evil($x) {
	global $txt;
	$txt[3] = "\xFF";
	var_dump($x);
	return $x[0];
}

$txt = "ola123";
var_dump(preg_replace_callback('#.#u', 'evil', $txt));
var_dump($txt);
var_dump(preg_last_error() == PREG_NO_ERROR);

var_dump(preg_replace_callback('#.#u', 'evil', $txt));
var_dump(preg_last_error() == PREG_BAD_UTF8_ERROR);

echo "Done!\n";
?>