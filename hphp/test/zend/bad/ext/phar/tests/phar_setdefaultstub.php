<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar';

$phar = new Phar($fname);
$phar['a.php'] = '<php echo "this is a\n"; ?>';
$phar['b.php'] = '<php echo "this is b\n"; ?>';
$phar->setDefaultStub();
$phar->stopBuffering();

var_dump($phar->getStub());

echo "============================================================================\n";
echo "============================================================================\n";

$phar->setDefaultStub('my/custom/thingy.php');
$phar->stopBuffering();
var_dump($phar->getStub());

echo "============================================================================\n";
echo "============================================================================\n";

$phar->setDefaultStub('my/custom/thingy.php', 'the/web.php');
$phar->stopBuffering();
var_dump($phar->getStub());

echo "============================================================================\n";
echo "============================================================================\n";

try {
	$phar->setDefaultStub(str_repeat('a', 400));
	$phar->stopBuffering();
	var_dump(strlen($phar->getStub()));

	$phar->setDefaultStub(str_repeat('a', 401));
	$phar->stopBuffering();
	var_dump(strlen($phar->getStub()));

} catch(Exception $e) {
	echo $e->getMessage() . "\n";
}

?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar');
?>