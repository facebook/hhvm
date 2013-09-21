<?php

$data = array('foo' => 123);

var_dump(
	filter_var_array($data, array('foo' => array('filter' => FILTER_DEFAULT), 'bar' => array('filter' => FILTER_DEFAULT)), false),
	filter_var_array($data, array('foo' => array('filter' => FILTER_DEFAULT), 'bar' => array('filter' => FILTER_DEFAULT)))
);

?>