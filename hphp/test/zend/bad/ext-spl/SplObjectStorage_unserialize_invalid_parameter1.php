<?php

$data_provider = array(
	array(),
	new stdClass(),
);

foreach($data_provider as $input) {

	$s = new SplObjectStorage();

	var_dump($s->unserialize($input));
}

?>