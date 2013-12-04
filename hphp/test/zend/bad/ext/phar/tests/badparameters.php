<?php
ini_set('phar.readonly', 1);
Phar::mungServer('hi');
Phar::createDefaultStub(array());
Phar::loadPhar(array());
Phar::canCompress('hi');
$a = new Phar(array());
$a = new Phar(dirname(__FILE__) . '/files/frontcontroller10.phar');
$a->convertToExecutable(array());
$a->convertToData(array());
$b = new PharData(dirname(__FILE__) . '/whatever.tar');
$c = new PharData(dirname(__FILE__) . '/whatever.zip');
$b->delete(array());
try {
$a->delete('oops');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$b->delete('oops');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
echo $a->getPath() . "\n";
try {
$a->setAlias('oops');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$b->setAlias('oops');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
ini_set('phar.readonly', 0);
$a->setAlias(array());
ini_set('phar.readonly', 1);
try {
$b->stopBuffering();
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$a->setStub('oops');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$b->setStub('oops');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
ini_set('phar.readonly', 0);
$a->setStub(array());
ini_set('phar.readonly', 1);
try {
$b->setDefaultStub('oops');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
$a->setDefaultStub(array());
try {
$a->setDefaultStub('oops');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$a->setSignatureAlgorithm(Phar::MD5);
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
$a->compress(array());
try {
$a->compress(1);
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
$a->compressFiles(array());
try {
$a->decompressFiles();
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
$a->copy(array());
try {
$a->copy('a', 'b');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
$a->offsetExists(array());
$a->offsetGet(array());
ini_set('phar.readonly', 0);
$a->offsetSet(array());
ini_set('phar.readonly', 1);
$b->offsetUnset(array());
try {
$a->offsetUnset('a');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
$a->addEmptyDir(array());
$a->addFile(array());
$a->addFromString(array());
try {
$a->setMetadata('a');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
ini_set('phar.readonly', 0);
$a->setMetadata(1,2);
ini_set('phar.readonly', 1);
try {
$a->delMetadata();
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
?>
===DONE===