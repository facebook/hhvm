<?php

$data_provider = array(
    new stdClass,
	array(),
	true,
	"string",
	12345,
	1.2345,
	NULL
);

foreach($data_provider as $input) {

	$h = new SplMaxHeap();

	var_dump($h->count($input));
}

?>