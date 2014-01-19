<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar';

$p = new Phar($fname);
try {
	$p->setAlias('hi/');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	$p->setAlias('hi\\l');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}

try {
	$p->setAlias('hil;');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}

try {
	$p->setAlias(':hil');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar');
?>