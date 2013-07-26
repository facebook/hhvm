<?php
try {
	$a = new PharData(dirname(__FILE__) . '/files/bzip2.zip');
	foreach ($a as $entry => $file) {
		echo $file->getContent();
	}
	$a = new Phar(dirname(__FILE__) . '/files/bz2_alias.phar.zip');
	var_dump($a->getAlias());
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===