<?php

file_put_contents('phar://' . dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php/a.php',
	'brand new!');

$phar = new Phar(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php');

var_dump($phar->getSignature());
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>