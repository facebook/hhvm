<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = '<?php echo "first stub\n"; __HALT_COMPILER(); ?>';

$files = array();
$files['a'] = 'a';
$files['b'] = 'b';
$files['c'] = 'c';

include 'files/phar_test.inc';

$phar = new Phar($fname);
echo $phar->getAlias() . "\n";
$phar->setAlias('test');
echo $phar->getAlias() . "\n";
$b = $phar;
$phar = new Phar(dirname(__FILE__) . '/notphar.phar');
try {
	$phar->setAlias('test');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	$b = new Phar(dirname(__FILE__) . '/nope.phar', 0, 'test');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phartmp.php');
unlink(dirname(__FILE__) . '/notphar.phar');
__HALT_COMPILER();
?>