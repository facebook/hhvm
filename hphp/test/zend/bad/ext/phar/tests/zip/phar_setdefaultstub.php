<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip';

$phar = new Phar($fname);
$phar['a.php'] = '<php echo "this is a\n"; ?>';
$phar['b.php'] = '<php echo "this is b\n"; ?>';
$phar->setStub('<?php echo "Hello World\n"; __HALT_COMPILER(); ?>');

var_dump($phar->getStub());

echo "============================================================================\n";
echo "============================================================================\n";

try {
	$phar->setDefaultStub();
	$phar->stopBuffering();
} catch(Exception $e) {
	echo $e->getMessage(). "\n";
}

var_dump($phar->getStub());

echo "============================================================================\n";
echo "============================================================================\n";

try {
	$phar->setDefaultStub('my/custom/thingy.php');
	$phar->stopBuffering();
} catch(Exception $e) {
	echo $e->getMessage(). "\n";
}

var_dump($phar->getStub());

echo "============================================================================\n";
echo "============================================================================\n";

try {
	$phar->setDefaultStub('my/custom/thingy.php', 'the/web.php');
	$phar->stopBuffering();
} catch(Exception $e) {
	echo $e->getMessage(). "\n";
}

var_dump($phar->getStub());

?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip');
?>