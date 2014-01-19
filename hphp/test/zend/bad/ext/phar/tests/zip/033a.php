<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip';
$alias = 'phar://hio';

$phar = new Phar($fname);
$phar['a.php'] = '<?php echo "This is a\n"; include "'.$alias.'/b.php"; ?>';
$phar->setAlias('hio');
$phar->addEmptyDir('test');
$phar->stopBuffering();
ini_set('phar.readonly', 1);

try {
	var_dump($phar['a.php']->isExecutable());
	$phar['a.php']->chmod(0777);
	var_dump($phar['a.php']->isExecutable());
	$phar['a.php']->chmod(0666);
	var_dump($phar['a.php']->isExecutable());
	echo "test dir\n";
	var_dump($phar['test']->isExecutable());
	$phar['test']->chmod(0777);
	var_dump($phar['test']->isExecutable());
	$phar['test']->chmod(0666);
	var_dump($phar['test']->isExecutable());
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip');
?>