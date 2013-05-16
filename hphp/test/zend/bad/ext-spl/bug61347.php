<?php
$a = array('b' => NULL, 37 => NULL);
var_dump(isset($a['b'])); //false

$b = new ArrayObject($a);
var_dump(isset($b['b'])); //false
var_dump(isset($b[37])); //false
var_dump(isset($b['no_exists'])); //false
var_dump(empty($b['b'])); //true
var_dump(empty($b[37])); //true

var_dump(array_key_exists('b', $b)); //true
var_dump($b['b']);

$a = array('b' => '', 37 => false);
$b = new ArrayObject($a);
var_dump(isset($b['b'])); //true
var_dump(isset($b[37])); //true
var_dump(isset($b['no_exists'])); //false
var_dump(empty($b['b'])); //true
var_dump(empty($b[37])); //true

