<?php
try {
	$phar = new Phar(dirname(__FILE__) . '/buildfromiterator.phar');
	$phar->buildFromIterator(new stdClass);
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