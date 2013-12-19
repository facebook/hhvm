<?php

$pharconfig = 2;

require_once 'files/phar_oo_test.inc';

$phar = new Phar($fname);
$phar->setInfoClass('SplFileObject');

$f = $phar['a.csv'];
$f->setFlags(SplFileObject::SKIP_EMPTY | SplFileObject::DROP_NEW_LINE);
foreach($f as $k => $v)
{
	echo "$k=>$v\n";
}

?>
===CSV===
<?php

$f->setFlags(SplFileObject::SKIP_EMPTY | SplFileObject::DROP_NEW_LINE | SplFileObject::READ_CSV);
foreach($f as $k => $v)
{
	echo "$k=>" . join('|', $v) . "\n";
}

?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/files/phar_oo_test.phar.php');
__halt_compiler();
?>