<?php

try
{
	$pharconfig = 0;
	
	require_once 'files/phar_oo_test.inc';
	
	$phar = new Phar($fname);
	
	$phar['f.php'] = 'hi';
	var_dump(isset($phar['f.php']));
	echo $phar['f.php'];
	echo "\n";
}
catch (BadMethodCallException $e)
{
	echo "Exception: " . $e->getMessage() . "\n";
}

?>
===DONE===
<?php error_reporting(0); ?>
<?php 
unlink(dirname(__FILE__) . '/files/phar_oo_011b.phar.php');
__halt_compiler();
?>