<?php
	var_dump(array_fill_keys('test', 1));
	var_dump(array_fill_keys(array(), 1));
	var_dump(array_fill_keys(array('foo', 'bar'), NULL));
	var_dump(array_fill_keys(array('5', 'foo', 10, 1.23), 123));
	var_dump(array_fill_keys(array('test', TRUE, 10, 100), ''));
?>