<?php
include dirname(__FILE__) . '/files/make_invalid_tar.php.inc';

$tar = new corrupter(dirname(__FILE__) . '/tar_001.phar.tar', 'none');
$tar->init();
$tar->addFile('tar_001.phpt', __FILE__);
$tar->close();

$tar = fopen('phar://' . dirname(__FILE__) . '/tar_001.phar.tar/tar_001.phpt', 'rb');
try {
	$phar = new Phar(dirname(__FILE__) . '/tar_001.phar.tar');
	echo "should not execute\n";
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php
@unlink(dirname(__FILE__) . '/tar_001.phar.tar');
?>