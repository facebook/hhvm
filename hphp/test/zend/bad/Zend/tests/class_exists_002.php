<?php

class foo {
	
}

var_dump(class_exists(''));
var_dump(class_exists(NULL));
var_dump(class_exists('FOO'));
var_dump(class_exists('bar'));
var_dump(class_exists(1));
var_dump(class_exists(new stdClass));

?>