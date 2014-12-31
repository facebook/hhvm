<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '2.phar.zip';
$fname3 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '3.phar.zip';

$phar = new Phar($fname);
$phar->setStub('<?php echo "first stub\n"; __HALT_COMPILER(); ?>');
$phar->setAlias('hio');

$files = array();

$files['a'] = 'a';
$files['b'] = 'b';
$files['c'] = 'c';

foreach ($files as $n => $file) {
	$phar[$n] = $file;
}
$phar->stopBuffering();

echo $phar->getAlias() . "\n";
$phar->setAlias('test');
echo $phar->getAlias() . "\n";

// test compression

$phar->compressFiles(Phar::GZ);
copy($fname, $fname2);
$phar->setAlias('unused');
$p2 = new Phar($fname2);
echo $p2->getAlias(), "\n";
$p2->compressFiles(Phar::BZ2);
copy($fname2, $fname3);
$p2->setAlias('unused2');
$p3 = new Phar($fname3);
echo $p3->getAlias(), "\n";
?>
===DONE===
<?php error_reporting(0); ?>
<?php
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '2.phar.zip');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '3.phar.zip');
__HALT_COMPILER();
?>