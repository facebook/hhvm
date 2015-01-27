<?php

$pharconfig = 1;

require_once 'files/phar_oo_test.inc';

$phar = new Phar($fname);
$phar->setInfoClass('SplFileObject');

$f = $phar['a.csv'];
echo "===1===\n";
foreach($f as $k => $v)
{
	echo "$k=>$v\n";
}

$f->setFlags(SplFileObject::DROP_NEW_LINE);

echo "===2===\n";
foreach($f as $k => $v)
{
	echo "$k=>$v\n";
}

class MyCSVFile extends SplFileObject
{
	function current()
	{
		return parent::fgetcsv(',', '"');
	}
}

$phar->setInfoClass('MyCSVFile');
$v = $phar['a.csv'];

echo "===3===\n";
while(!$v->eof())
{
	echo $v->key() . "=>" . join('|',$v->fgetcsv()) . "\n";
}

echo "===4===\n";
$v->rewind();
while(!$v->eof())
{
	$l = $v->fgetcsv();
	echo $v->key() . "=>" . join('|',$l) . "\n";
}

echo "===5===\n";
foreach($v as $k => $d)
{
	echo "$k=>" . join('|',$d) . "\n";
}

class MyCSVFile2 extends SplFileObject
{
	function getCurrentLine()
	{
		echo __METHOD__ . "\n";
		return parent::fgetcsv(',', '"');
	}
}

$phar->setInfoClass('MyCSVFile2');
$v = $phar['a.csv'];

echo "===6===\n";
foreach($v as $k => $d)
{
	echo "$k=>" . join('|',$d) . "\n";
}

?>
===DONE===
<?php error_reporting(0); ?>
<?php 
unlink(dirname(__FILE__) . '/files/phar_oo_008.phar.php');
__halt_compiler();
?>