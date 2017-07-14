<?php

function f(...$args) {
	var_dump(count($args));
}
(function(){
	$a = array_fill(0, 1024, true);
	f(...$a);
	yield;
})()->valid();

?>
