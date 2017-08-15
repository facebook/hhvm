<?php

$g = (function($a) {
	try {
		return $a + 1;
	} finally {
		$b = $a + 2;
		var_dump($b);
	}
	yield; // Generator
})(1);
$g->next();
var_dump($g->getReturn());

?>
