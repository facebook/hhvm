<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar';
$pname = 'phar://' . $fname;

include dirname(__FILE__) . '/files/corrupt_tarmaker.php.inc';
$a = new corrupt_tarmaker($fname, 'none');
$a->init();
$a->addFile('hithere', 'contents', null, 'checksum');
$a->close();

try {
	$p = new PharData($fname);
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}

?>
===DONE===
<?php
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar');
?>