<?php
try {
	chdir(dirname(__FILE__));
	$phar = new Phar(dirname(__FILE__) . '/buildfromiterator10.phar');
	$dir = new RecursiveDirectoryIterator('.');
	$iter = new RecursiveIteratorIterator($dir);
	$a = $phar->buildFromIterator(new RegexIterator($iter, '/_\d{3}\.phpt$/'), dirname(__FILE__) . DIRECTORY_SEPARATOR);
	asort($a);
	var_dump($a);
} catch (Exception $e) {
	var_dump(get_class($e));
	echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php error_reporting(0); ?>
<?php 
unlink(dirname(__FILE__) . '/buildfromiterator10.phar');
__HALT_COMPILER();
?>