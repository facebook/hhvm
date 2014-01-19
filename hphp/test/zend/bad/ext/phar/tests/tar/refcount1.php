<?php
include dirname(__FILE__) . '/files/tarmaker.php.inc';
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar';
$alias = 'phar://' . $fname;

$tar = new tarmaker($fname, 'none');
$tar->init();
$tar->addFile('.phar/stub.php', "<?php __HALT_COMPILER(); ?>");

$files = array();

$files['a.php'] = '<?php echo "This is a\n"; ?>';
$files['b.php'] = '<?php echo "This is b\n"; ?>';
$files['b/c.php'] = '<?php echo "This is b/c\n"; ?>';
$files['.phar/alias.txt'] = 'hio';

foreach ($files as $n => $file) {
	$tar->addFile($n, $file);
}

$tar->close();

$fp = fopen($alias . '/b/c.php', 'wb');
fwrite($fp, b"extra");
fclose($fp);
echo "===CLOSE===\n";
$phar = new Phar($fname);
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
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar'); ?>