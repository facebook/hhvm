<?php

$s = new SplObjectStorage();
$o1 = new stdClass();

try {
	$s->offsetGet($o1);
} catch (UnexpectedValueException $e) {
	echo $e->getMessage();
}

?>