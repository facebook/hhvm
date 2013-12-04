<?php

require_once 'files/phar_oo_test.inc';

class MyFile extends SplFileObject
{
	function __construct($what)
	{
		echo __METHOD__ . "($what)\n";
		parent::__construct($what);
	}
}

$phar = new Phar($fname);
try
{
	$phar->setFileClass('SplFileInfo');
}
catch (UnexpectedValueException $e)
{
	echo $e->getMessage() . "\n";
}
$phar->setInfoClass('MyFile');

echo $phar['a.php']->getFilename() . "\n";
echo $phar['b/c.php']->getFilename() . "\n";
echo $phar['b.php']->getFilename() . "\n";

?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/files/phar_oo_test.phar.php');
__halt_compiler();
?>