<?php
// numeric keys
$input = array('test', 'foo', 'bar');
$output = bson_encode($input);
$testValue = bson_decode($output);
var_dump($testValue);

// string keys
$input = array('test' => 'test', 'foo' => 'foo', 'bar' => 'bar');
$output = bson_encode($input);
$testValue = bson_decode($output);
var_dump($testValue);

// mixed keys
$input = array('foo' => 'test', 'foo', 'bar');
$output = bson_encode($input);
$testValue = bson_decode($output);
var_dump($testValue);
?>
