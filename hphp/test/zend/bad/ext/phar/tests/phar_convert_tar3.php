<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar';
$fname3 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.tar';
$stub = '<?php echo "first stub\n"; __HALT_COMPILER(); ?>';
$file = $stub;

$files = array();
$files['a'] = 'a';
$files['b'] = 'b';
$files['c'] = 'c';

include 'files/phar_test.inc';

$phar = new Phar($fname);
var_dump($phar->isFileFormat(Phar::TAR));
var_dump($phar->isCompressed());
var_dump($phar->getStub());

$phar = $phar->convertToExecutable(Phar::TAR, Phar::BZ2);
var_dump($phar->isFileFormat(Phar::TAR));
var_dump($phar->isCompressed());
var_dump($phar->getStub());

copy($fname2 . '.bz2', $fname3);

$phar = new Phar($fname3);
var_dump($phar->isFileFormat(Phar::TAR));
var_dump($phar->isCompressed() == Phar::BZ2);
var_dump($phar->getStub());

?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar.bz2');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.tar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php');
__HALT_COMPILER();
?>