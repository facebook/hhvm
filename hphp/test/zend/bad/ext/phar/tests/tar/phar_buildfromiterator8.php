<?php
try {
	chdir(dirname(__FILE__));
	$phar = new Phar(dirname(__FILE__) . '/buildfromiterator.phar.tar');
	$a = $phar->buildFromIterator(new RegexIterator(new DirectoryIterator('.'), '/^frontcontroller\d{0,2}\.phar\.phpt\\z|^\.\\z|^\.\.\\z/'), dirname(__FILE__) . DIRECTORY_SEPARATOR);
	asort($a);
	var_dump($a);
	var_dump($phar->isFileFormat(Phar::TAR));
} catch (Exception $e) {
	var_dump(get_class($e));
	echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/buildfromiterator.phar.tar');
__HALT_COMPILER();
?>