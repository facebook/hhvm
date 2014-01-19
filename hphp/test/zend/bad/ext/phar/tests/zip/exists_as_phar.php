<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar';
$tname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip';
$alias = 'phar://hio';

$phar = new Phar($fname);
$phar['a.php'] = '<?php echo "This is a\n"; include "'.$alias.'/b.php"; ?>';
$phar->setAlias('hio');
$phar->addEmptyDir('test');
$phar->stopBuffering();
copy($fname, $tname);
$phar->setAlias('hio2');

try {
	$p = new Phar($tname);
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}

?>
===DONE===
<?php
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar');
?>