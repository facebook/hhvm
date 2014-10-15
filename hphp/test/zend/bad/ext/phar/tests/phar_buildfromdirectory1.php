<?php
$phar = new Phar(dirname(__FILE__) . '/buildfromdirectory1.phar');
try {
	ini_set('phar.readonly', 1);
	$phar->buildFromDirectory(1);
} catch (Exception $e) {
	var_dump(get_class($e));
	echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php error_reporting(0); ?>
<?php 
unlink(dirname(__FILE__) . '/buildfromdirectory1.phar');
__HALT_COMPILER();
?>