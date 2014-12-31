<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar';
$fname3 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.3.phar';

$phar = new Phar($fname);
$phar['a.txt'] = 'some text';
$phar->stopBuffering();
var_dump($phar->isFileFormat(Phar::TAR));
var_dump(strlen($phar->getStub()));

$phar = $phar->convertToExecutable(Phar::TAR);
var_dump($phar->isFileFormat(Phar::TAR));
var_dump($phar->getStub());

$phar['a'] = 'hi there';

$phar = $phar->convertToExecutable(Phar::PHAR, Phar::NONE, '.3.phar');
var_dump($phar->isFileFormat(Phar::PHAR));
var_dump(strlen($phar->getStub()));

copy($fname3, $fname2);

$phar = new Phar($fname2);
var_dump($phar->isFileFormat(Phar::PHAR));
var_dump(strlen($phar->getStub()));

?>
===DONE===
<?php error_reporting(0); ?>
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.3.phar');
__HALT_COMPILER();
?>