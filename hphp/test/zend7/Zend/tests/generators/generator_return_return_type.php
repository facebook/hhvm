<?php

$gen = (function (): Generator {
	1 + $a = 1; // force a temporary
	return true;
	yield;
})();

var_dump($gen->valid());
var_dump($gen->getReturn());

?>
