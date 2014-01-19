<?php

require_once 'files/phar_oo_test.inc';

$phar = new Phar($fname);
$phar->setInfoClass('SplFileInfo');
foreach($phar as $name => $ent)
{
	var_dump(str_replace(str_replace('\\', '/', dirname(__FILE__)), '*', $name));
	var_dump($ent->getFilename());
	var_dump($ent->getSize());
	var_dump($ent->getType());
	var_dump($ent->isWritable());
	var_dump($ent->isReadable());
	var_dump($ent->isExecutable());
	var_dump($ent->isFile());
	var_dump($ent->isDir());
	var_dump($ent->isLink());
	var_dump($ent->getCTime());
	var_dump($ent->getMTime());
	var_dump($ent->getATime());
}

echo "==RECURSIVE==\n";

$phar = new Phar($fname);
foreach(new RecursiveIteratorIterator($phar) as $name => $ent)
{
	var_dump(str_replace(str_replace('\\', '/', dirname(__FILE__)), '*', $name));
	var_dump(str_replace('\\', '/', $ent->getFilename()));
	var_dump($ent->getCompressedSize());
	var_dump($ent->isCRCChecked());
	var_dump($ent->isCRCChecked() ? $ent->getCRC32() : NULL);
	var_dump($ent->getPharFlags());
}

?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/files/phar_oo_test.phar.php');
__halt_compiler();
?>