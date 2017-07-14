<?php

function getNumber() : int {
	return "foo";
}

try {
	getNumber();
} catch (TypeError $e) {
	var_dump($e->getMessage());
}
?>
