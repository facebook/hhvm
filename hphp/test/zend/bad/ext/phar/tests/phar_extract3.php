<?php

$fname   = dirname(__FILE__) . '/files/bogus.zip';
$fname2  = dirname(__FILE__) . '/files/notbogus.zip';
$extract = dirname(__FILE__) . '/test-extract3';

$phar = new PharData($fname);

try {
	$phar->extractTo($extract);
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}

$phar = new PharData($fname2);
foreach ($phar as $filename) {
	echo "$filename\n";
}

try {
	$phar->extractTo($extract);
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}

?>
===DONE===
<?php error_reporting(0); ?>
<?php
$dir = dirname(__FILE__) . '/test-extract3/';
@unlink($dir . 'stuff.txt');
@unlink($dir . 'nonsense.txt');
@rmdir($dir);
?>