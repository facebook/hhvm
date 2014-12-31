<?php
include dirname(__FILE__) . '/files/tarmaker.php.inc';
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar';
$alias = 'phar://' . $fname;

$tar = new tarmaker($fname, 'none');
$tar->init();
$tar->addFile('.phar/stub.php', "<?php
Phar::mapPhar('hio');
__HALT_COMPILER(); ?>");

$files = array();
$files['a/x'] = 'a';

foreach ($files as $n => $file) {
	$tar->addFile($n, $file);
}
$tar->mkdir('a');

$tar->close();

include $fname;

echo file_get_contents($alias . '/a/x') . "\n";
var_dump(rmdir($alias . '/a'));
echo file_get_contents($alias . '/a/x') . "\n";
unlink($alias . '/a/x');
var_dump(rmdir($alias . '/a'));
?>
<?php error_reporting(0); ?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar'); ?>