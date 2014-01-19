<?php

file_put_contents('phar://' . dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip/a.php',
	'brand new!');
include 'phar://' . dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip/a.php';
?>

===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip'); ?>