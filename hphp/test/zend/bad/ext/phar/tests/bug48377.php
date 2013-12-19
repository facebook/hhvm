<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.zip';

touch($fname2);

$phar = new Phar($fname, 0, 'a.phar');
$phar['x'] = 'hi';
try {
	$phar->convertToData(Phar::ZIP, Phar::NONE, 'zip');
} catch (BadMethodCallException $e) {
	echo $e->getMessage(),"\n";
}
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar');?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.zip');?>