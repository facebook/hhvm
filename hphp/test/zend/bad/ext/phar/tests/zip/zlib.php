<?php
try {
	$a = new Phar(dirname(__FILE__) . '/files/zlib_alias.phar.zip');
	var_dump($a->getAlias());
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===