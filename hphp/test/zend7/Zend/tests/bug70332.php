<?php
function & test($arg) {
	return $arg;
}

$arg = new Stdclass();
$arg->name = array();

test($arg)->name[1] = "xxxx";

print_r($arg);
?>
