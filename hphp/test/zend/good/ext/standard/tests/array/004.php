<?php
$data = array(
	'Test1',
	'teST2'=>0,
	5=>'test2',
	'abc'=>'test10',
	'test21'
);

var_dump($data);

natsort($data);
var_dump($data);

natcasesort($data);
var_dump($data);
?>