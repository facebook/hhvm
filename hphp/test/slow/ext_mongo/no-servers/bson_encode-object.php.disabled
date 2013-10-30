<?php

$string = '3';

$input = new stdClass();
$input->int = 3;
$input->boolean = true;
$input->array = array('foo', 'bar');
$input->object = new stdClass();
$input->string = 'test';
$input->$string = 'test';

$output = bson_encode($input);
$testValue = bson_decode($output);
var_dump($testValue);

?>
