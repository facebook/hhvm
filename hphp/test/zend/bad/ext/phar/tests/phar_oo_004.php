<?php

require_once 'files/phar_oo_test.inc';

$it = new DirectoryIterator('phar://'.$fname);

foreach($it as $name => $ent)
{
	var_dump($name);
	var_dump($ent->getFilename());
	var_dump($ent->isDir());
	var_dump($ent->isDot());
}

?>
===MANUAL===
<?php

class MyDirectoryIterator extends DirectoryIterator
{
	function __construct($dir)
	{
		echo __METHOD__ . "\n";
		parent::__construct($dir);
	}

	function rewind()
	{
		echo __METHOD__ . "\n";
		parent::rewind();
	}

	function valid()
	{
		echo __METHOD__ . "\n";
		return parent::valid();
	}

	function key()
	{
		echo __METHOD__ . "\n";
		return parent::key();
	}

	function current()
	{
		echo __METHOD__ . "\n";
		return parent::current();
	}

	function next()
	{
		echo __METHOD__ . "\n";
		parent::next();
	}
}

$it = new MyDirectoryIterator('phar://'.$fname);

foreach($it as $name => $ent)
{
	var_dump($name);
	var_dump($ent->getFilename());
}

?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/files/phar_oo_test.phar.php');
__halt_compiler();
?>