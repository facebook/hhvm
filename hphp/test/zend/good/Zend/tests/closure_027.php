<?php

function test(closure $a) {
	var_dump($a());
}


test(function() { return new stdclass; });

test(function() { });

$a = function($x) use ($y) {};
try { test($a); } catch (Exception $e) { var_dump($e->getMessage()); }

test(new stdclass);

