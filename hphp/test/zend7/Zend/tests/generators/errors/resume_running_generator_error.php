<?php

function gen() {
    $gen = yield;
    try {
	    $gen->next();
	} catch (Error $e) {
		echo "\nException: " . $e->getMessage() . "\n";
	}
	$gen->next();
}

$gen = gen();
$gen->send($gen);
$gen->next();

?>
