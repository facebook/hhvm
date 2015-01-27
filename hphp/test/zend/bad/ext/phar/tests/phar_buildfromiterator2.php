<?php
try {
	$phar = new Phar(dirname(__FILE__) . '/buildfromiterator2.phar');
	$phar->buildFromIterator(new stdClass);
} catch (Exception $e) {
	var_dump(get_class($e));
	echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php error_reporting(0); ?>
<?php 
unlink(dirname(__FILE__) . '/buildfromiterator2.phar');
__HALT_COMPILER();
?>