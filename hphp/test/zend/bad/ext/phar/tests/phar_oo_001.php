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
catch (LogicException $e)
{
	var_dump($e->getMessage());
}
try {
	$phar = new Phar('test.phar');
	$phar->__construct('oops');
} catch (LogicException $e)
{
	var_dump($e->getMessage());
}

?>
===DONE===
<?php error_reporting(0); ?>
<?php 
unlink(dirname(__FILE__) . '/files/phar_oo_001.phar.php');
__halt_compiler();
?>