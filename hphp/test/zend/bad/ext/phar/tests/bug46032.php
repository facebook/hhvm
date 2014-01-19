<?php

$a = dirname(__FILE__) .'/mytest';

try {
	new phar($a);
} catch (exception $e) { }

var_dump($a);

try {
	new phar($a);
} catch (exception $e) { }

var_dump($a);

new phardata('0000000000000000000');
?>
===DONE===