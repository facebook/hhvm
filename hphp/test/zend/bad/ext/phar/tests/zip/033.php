<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.zip';
$alias = 'phar://hio';

$phar = new Phar($fname);
$phar['a.php'] = '<?php echo "This is a\n"; include "'.$alias.'/b.php"; ?>';
$phar->setAlias('hio');
$phar->addEmptyDir('test');
$phar->stopBuffering();

try {
	var_dump($phar['a.php']->isExecutable());
	$phar['a.php']->chmod(0777);
	copy($fname, $fname2);
	$phar->setAlias('unused');
	$phar2 = new Phar($fname2);
	var_dump($phar2['a.php']->isExecutable());
	$phar['a.php']->chmod(0666);
	var_dump($phar['a.php']->isExecutable());
	echo "test dir\n";
	var_dump($phar['test']->isDir());
	var_dump($phar['test']->isReadable());
	$phar['test']->chmod(0000);
	var_dump($phar['test']->isReadable());
	$phar['test']->chmod(0666);
	var_dump($phar['test']->isReadable());
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.zip');
?>