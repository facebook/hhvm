<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.zip';

echo "=================== new PharData() ==================\n";
$phar = new PharData($fname);
$phar['a'] = 'a';
$phar['b'] = 'b';
$phar['c'] = 'c';

var_dump($phar->isFileFormat(Phar::PHAR));
var_dump($phar->isFileFormat(Phar::TAR));
var_dump($phar->isFileFormat(Phar::ZIP));
var_dump($phar->getStub());
var_dump($phar->getAlias());

echo "================= convertToTar() =====================\n";

$phar = $phar->convertToData(Phar::TAR);
var_dump($phar->isFileFormat(Phar::PHAR));
var_dump($phar->isFileFormat(Phar::TAR));
var_dump($phar->isFileFormat(Phar::ZIP));
var_dump($phar->getStub());
var_dump($phar->getAlias());

echo "================= convertToZip() =====================\n";

$phar = $phar->convertToData(Phar::ZIP, Phar::NONE, '.1.zip');
var_dump($phar->isFileFormat(Phar::PHAR));
var_dump($phar->isFileFormat(Phar::TAR));
var_dump($phar->isFileFormat(Phar::ZIP));
var_dump($phar->getStub());
var_dump($phar->getAlias());

echo "================= convertToPhar() ====================\n";

try {
	$phar = $phar->convertToExecutable(Phar::PHAR);
	var_dump($phar->isFileFormat(Phar::PHAR));
	var_dump($phar->isFileFormat(Phar::TAR));
	var_dump($phar->isFileFormat(Phar::ZIP));
	var_dump(strlen($phar->getStub()));
	var_dump($phar->getAlias());
} catch(Exception $e) {
	echo $e->getMessage()."\n";
}

echo "================ convertToTar(GZ) ====================\n";

$phar = $phar->convertToData(Phar::TAR, Phar::GZ, '.2.tar');
var_dump($phar->isFileFormat(Phar::PHAR));
var_dump($phar->isFileFormat(Phar::TAR));
var_dump($phar->isFileFormat(Phar::ZIP));
var_dump($phar->getStub());
var_dump($phar->getAlias());

echo "================= convertToPhar() ====================\n";

try {
	$phar = $phar->convertToExecutable(Phar::PHAR);
	var_dump($phar->isFileFormat(Phar::PHAR));
	var_dump($phar->isFileFormat(Phar::TAR));
	var_dump($phar->isFileFormat(Phar::ZIP));
	var_dump(strlen($phar->getStub()));
	var_dump($phar->getAlias());
} catch(Exception $e) {
	echo $e->getMessage()."\n";
}

?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.gz');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar.gz');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.tar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.zip');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.1.zip');
?>