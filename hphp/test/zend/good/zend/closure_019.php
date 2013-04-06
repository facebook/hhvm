<?php

$lambda = function &(&$x) {
	return $x = $x * $x;
};

function test() {
	global $lambda;
	
	$y = 3;
	var_dump($GLOBALS['lambda']($y));
	var_dump($lambda($y));
	var_dump($GLOBALS['lambda'](1));
}

test();

?>