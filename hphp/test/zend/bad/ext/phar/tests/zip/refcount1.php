<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip';
$alias = 'phar://' . $fname;

$phar = new Phar($fname);
$phar->setStub("<?php __HALT_COMPILER(); ?>");
$phar->setAlias('hio');

$files = array();

$files['a.php'] = '<?php echo "This is a\n"; ?>';
$files['b.php'] = '<?php echo "This is b\n"; ?>';
$files['b/c.php'] = '<?php echo "This is b/c\n"; ?>';

foreach ($files as $n => $file) {
	$phar[$n] = $file;
}
$phar->stopBuffering();

$fp = fopen($alias . '/b/c.php', 'wb');
fwrite($fp, b"extra");
fclose($fp);
echo "===CLOSE===\n";
$b = fopen($alias . '/b/c.php', 'rb');
$a = $phar['b/c.php'];
var_dump($a);
var_dump(fread($b, 20));
rewind($b);
echo "===UNLINK===\n";
unlink($alias . '/b/c.php');
var_dump($a);
var_dump(fread($b, 20));
include $alias . '/b/c.php';
?>

===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip'); ?>