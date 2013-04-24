<?php

$test = array(
	'a'     => '1',
	'æ'     => '2',
	'æøåäö' => '3',
);

var_dump($test);
var_dump(extract($test));
var_dump($a);
var_dump($æ);
var_dump($æøåäö);

echo "Done.\n";
?>