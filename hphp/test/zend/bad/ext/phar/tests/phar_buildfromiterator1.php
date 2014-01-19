<?php
$phar = new Phar(dirname(__FILE__) . '/buildfromiterator.phar');
try {
	ini_set('phar.readonly', 1);
	$phar->buildFromIterator(1);
} catch (Exception $e) {
	var_dump(get_class($e));
	echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/buildfromiterator.phar');
__HALT_COMPILER();
?>