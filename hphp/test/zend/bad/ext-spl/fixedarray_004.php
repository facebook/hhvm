<?php

$a = new SplFixedArray(10);

try {
	$a[] = 1;
} catch (Exception $e) {
	var_dump($e->getMessage());
}

?>
===DONE===