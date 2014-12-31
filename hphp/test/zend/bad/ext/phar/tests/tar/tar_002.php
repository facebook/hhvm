<?php
include dirname(__FILE__) . '/files/make_invalid_tar.php.inc';

$tar = new corrupter(dirname(__FILE__) . '/tar_002.phar.tar', 'none');
$tar->init();
$tar->addFile('tar_002.phpt', __FILE__);
$tar->close();

$tar = fopen('phar://' . dirname(__FILE__) . '/tar_002.phar.tar/tar_002.phpt', 'rb');

try {
	$phar = new Phar(dirname(__FILE__) . '/tar_002.phar.tar');
	echo "should not execute\n";
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php error_reporting(0); ?>
<?php
@unlink(dirname(__FILE__) . '/tar_002.phar.tar');
?>