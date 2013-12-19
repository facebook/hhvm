<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.zip';

$phar = new PharData($fname);
$phar['x'] = 'hi';
try {
	$phar->convertToData(Phar::ZIP, Phar::NONE, '.2.phar.zip');
} catch (BadMethodCallException $e) {
	echo $e->getMessage(),"\n";
}
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.zip');?>