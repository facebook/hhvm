<?php
try {
	$phar = new Phar(dirname(__FILE__) . '/buildfromdirectory.phar');
	$phar->buildFromDirectory(1);
} catch (Exception $e) {
	var_dump(get_class($e));
	echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/buildfromdirectory.phar');
__HALT_COMPILER();
?>