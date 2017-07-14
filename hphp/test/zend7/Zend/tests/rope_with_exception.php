<?php

class Obj {
	function __get($x) {
		throw new Exception();
	}
}

try {
	$x = new Obj;
	$y = 0;
	$r = "$y|$x->x|";
	echo "should never be reached";
} catch (Exception $e) {
	echo "$e\n";
}

try {
	$x = new Obj;
	$y = 0;
	$r = "$y$x->x|";
	echo "should never be reached";
} catch (Exception $e) {
	echo "$e\n";
}

try {
	$x = new Obj;
	$y = 0;
	$r = "$y|$y$x->x";
	echo "should never be reached";
} catch (Exception $e) {
	echo "$e\n";
}

?>
