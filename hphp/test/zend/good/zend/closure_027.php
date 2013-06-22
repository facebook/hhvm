<?php

function test(closure $a) {
	var_dump($a());
}


test(function() { return new stdclass; });

test(function() { });

$a = function($x) use ($y) {};
test($a);

test(new stdclass);

?>