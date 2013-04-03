<?php
$func = create_function('','
	static $foo = 0;
	return $foo++;
');
var_dump($func());
var_dump($func());
var_dump($func());
?>