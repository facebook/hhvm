<?php

require_once 'files/phar_oo_test.inc';

$phar = new Phar($fname);
var_dump($phar->getVersion());
var_dump(count($phar));

class MyPhar extends Phar
{
	function __construct()
	{
	}
}

try
{
	$phar = new MyPhar();
	var_dump($phar->getVersion());
}
catch (BadMethodCallException $e)
{
	var_dump($e->getMessage());
}
try {
	$phar = new Phar('test.phar');
	$phar->__construct('oops');
} catch (BadMethodCallException $e)
{
	var_dump($e->getMessage());
}

?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/files/phar_oo_test.phar.php');
__halt_compiler();
?>