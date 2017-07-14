<?php

function test(array... $args) {
    var_dump($args);
}

try {
	test([0], [1], 2);
} catch(Error $e) {
	var_dump($e->getMessage());
}

?>
