<?php
$array = array('expected_array' => "foobar");
var_dump(isset($array['expected_array']));
var_dump($array['expected_array']);
var_dump(isset($array['expected_array']['foo']));
var_dump($array['expected_array']['foo']);
var_dump(isset($array['expected_array']['foo']['bar']));
var_dump($array['expected_array']['foo']['bar']);
?>