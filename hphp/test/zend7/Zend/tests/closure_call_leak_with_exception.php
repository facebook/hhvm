<?php

(function() {
	$closure = function($foo) { var_dump($foo); };
	$closure(yield);
})()->valid(); // start

?>
==DONE==
