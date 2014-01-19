<?php
ini_set('phar.require_hash', 1);
include dirname(__FILE__) . '/files/tarmaker.php.inc';
$fname = dirname(__FILE__) . '/tar_004.phar.tar';
$alias = 'phar://' . $fname;
$fname2 = dirname(__FILE__) . '/tar_004.tar';

$tar = new tarmaker($fname, 'none');
$tar->init();
$tar->addFile('tar_004.php', '<?php var_dump(__FILE__);');
$tar->addFile('internal/file/here', "hi there!\n");
$tar->addFile('.phar/stub.php', "__HALT_COMPILER();");
$tar->close();

try {
	$phar = new Phar($fname);
	var_dump($phar->getStub());
} catch (Exception $e) {
	echo $e->getMessage()."\n";
}
ini_set('phar.require_hash', 0);
try {
	$phar = new PharData($fname2);
	$phar['file'] = 'hi';
	var_dump($phar->getSignature());
	$phar->setSignatureAlgorithm(Phar::MD5);
	var_dump($phar->getSignature());
} catch (Exception $e) {
	echo $e->getMessage()."\n";
}

?>
===DONE===
<?php
@unlink(dirname(__FILE__) . '/tar_004.phar.tar');
@unlink(dirname(__FILE__) . '/tar_004.tar');
?>