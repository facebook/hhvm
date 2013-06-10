<?php
function callback($var) {
	return $var;
}
$var = "test";
var_dump(filter_var($var, FILTER_CALLBACK, array('options'=>'callback')));
?>