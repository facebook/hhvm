<?php
include dirname(__FILE__) . '/files/tarmaker.php.inc';
$fname = dirname(__FILE__) . '/tar_004.phar.tar';
$alias = 'phar://' . $fname;
$fname2 = dirname(__FILE__) . '/tar_004.tar';

$tar = new tarmaker($fname, 'none');
$tar->init();
$tar->addFile('tar_004.php', '<?php var_dump(__FILE__);');
$tar->addFile('internal/file/here', "hi there!\n");
$tar->close();

try {
	$phar = new Phar($fname);
	var_dump($phar->getStub());
} catch (Exception $e) {
	echo $e->getMessage()."\n";
}

copy($fname, $fname2);

try {
	$phar = new PharData($fname2);
	var_dump($phar->getStub());
} catch (Exception $e) {
	echo $e->getMessage()."\n";
}

?>
===DONE===
<?php error_reporting(0); ?>
<?php
@unlink(dirname(__FILE__) . '/tar_004.phar.tar');
@unlink(dirname(__FILE__) . '/tar_004.tar');
?>