<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '2.phar';

$phar = new Phar($fname);
$phar['a.txt'] = 'some text';
$phar->stopBuffering();
var_dump($phar->isFileFormat(Phar::TAR));
var_dump(strlen($phar->getStub()));

$phar = $phar->convertToExecutable(Phar::TAR);
var_dump($phar->isFileFormat(Phar::TAR));
var_dump($phar->getStub());

$phar['a'] = 'hi there';

$phar = $phar->convertToExecutable(Phar::PHAR, Phar::BZ2);
var_dump($phar->isFileFormat(Phar::PHAR));
var_dump($phar->isCompressed());
var_dump(strlen($phar->getStub()));

copy($fname . '.bz2', $fname2);

$phar = new Phar($fname2);
var_dump($phar->isFileFormat(Phar::PHAR));
var_dump($phar->isCompressed() == Phar::BZ2);
var_dump(strlen($phar->getStub()));

?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.bz2');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '2.phar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar.bz2');
__HALT_COMPILER();
?>