<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar';
$alias = 'phar://' . $fname;

$phar = new Phar($fname);
$phar->setStub("<?php __HALT_COMPILER(); ?>");

$files = array();

$files['a.php'] = '<?php echo "This is a\n"; ?>';
$files['b.php'] = '<?php echo "This is b\n"; ?>';
$files['b/c.php'] = '<?php echo "This is b/c\n"; ?>';

foreach ($files as $n => $file) {
	$phar[$n] = $file;
}

$phar->stopBuffering();
ini_set('phar.readonly', 1);

$fp = fopen($alias . '/b/new.php', 'wb');
fwrite($fp, 'extra');
fclose($fp);
include $alias . '/b/c.php';
include $alias . '/b/new.php';

?>

===DONE===
<?php error_reporting(0); ?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar'); ?>