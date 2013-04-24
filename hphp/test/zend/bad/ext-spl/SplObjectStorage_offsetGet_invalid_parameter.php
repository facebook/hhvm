<?php

$data_provider = array(
	array(),
	true,
	"string",
	12345,
	1.2345,
	NULL
);

foreach($data_provider as $input) {

	$s = new SplObjectStorage();
	$o1 = new stdClass();
	$s[$o1] = 'some_value';

	var_dump($s->offsetGet($input));
}

?>