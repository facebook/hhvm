<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip';

$p = new Phar($fname);
try {
	$p['.phar/stub.php'] = 'hi';
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	$p['.phar/alias.txt'] = 'hi';
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
$p = new Phar($fname2);
try {
	$p['.phar/stub.php'] = 'hi';
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	$p['.phar/alias.txt'] = 'hi';
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}

?>
===DONE===
<?php
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip');
?>