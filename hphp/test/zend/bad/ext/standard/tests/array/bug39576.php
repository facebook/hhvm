<?php

class Test {

	public $_table = '';
	public $_columns = array ();
	public $_primary = array ();

}

$test = new Test ();
$test->name = 'test';
$test->_columns['name'] = new stdClass;

function test ($value, $column, &$columns) {}

array_walk (
	get_object_vars ($test),
	'test',
	$test->_columns
);

var_dump($test);

array_intersect_key (
	get_object_vars ($test),
	$test->_primary
);

echo "Done\n";
?>