<?php

set_error_handler(function($_, $msg, $file) {
	var_dump($msg, $file);
	echo $undefined;
});

eval('class A { function a() {} function __construct() {} }');

?>