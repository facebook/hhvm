<?php
$phar = new Phar(dirname(__FILE__) . '/buildfromiterator1.phar');
try {
	ini_set('phar.readonly', 1);
	$phar->buildFromIterator(1);
} catch (Exception $e) {
	var_dump(get_class($e));
	echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php error_reporting(0); ?>
<?php 
unlink(dirname(__FILE__) . '/buildfromiterator1.phar');
__HALT_COMPILER();
?>