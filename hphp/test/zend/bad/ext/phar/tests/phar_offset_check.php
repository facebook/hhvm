<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://'.$fname;

$phar = new Phar($fname);
$phar->setDefaultStub();
$phar->setAlias('susan');
$phar['a.txt'] = "first file\n";
$phar['b.txt'] = "second file\n";

try {
	$phar->offsetGet('.phar/stub.php');
} catch (Exception $e) {
	echo $e->getMessage()."\n";
}

try {
	$phar->offsetGet('.phar/alias.txt');
} catch (Exception $e) {
	echo $e->getMessage()."\n";
}

try {
	$phar->offsetSet('.phar/stub.php', '<?php __HALT_COMPILER(); ?>');
} catch (Exception $e) {
	echo $e->getMessage()."\n";
}

var_dump(strlen($phar->getStub()));

try {
	$phar->offsetUnset('.phar/stub.php');
} catch (Exception $e) {
	echo $e->getMessage()."\n";
}

var_dump(strlen($phar->getStub()));

try {
	$phar->offsetSet('.phar/alias.txt', 'dolly');
} catch (Exception $e) {
	echo $e->getMessage()."\n";
}

var_dump($phar->getAlias());

try {
	$phar->offsetUnset('.phar/alias.txt');
} catch (Exception $e) {
	echo $e->getMessage()."\n";
}

var_dump($phar->getAlias());

?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>