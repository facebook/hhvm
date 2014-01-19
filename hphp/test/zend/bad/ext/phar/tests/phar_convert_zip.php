<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar';
$pname = 'phar://' . $fname;
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip';
$fname3 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.zip';
$stub = '<?php echo "first stub\n"; __HALT_COMPILER(); ?>';
$file = $stub;

$files = array();
$files['a'] = 'a';
$files['b'] = 'b';
$files['c'] = 'c';

include 'files/phar_test.inc';

$phar = new Phar($fname);
var_dump($phar->isFileFormat(Phar::ZIP));
var_dump($phar->getStub());

$phar = $phar->convertToExecutable(Phar::ZIP);
var_dump($phar->isFileFormat(Phar::ZIP));
var_dump($phar->getStub());

copy($fname2, $fname3);

$phar = new Phar($fname3);
var_dump($phar->isFileFormat(Phar::ZIP));
var_dump($phar->getStub());
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.zip');
__HALT_COMPILER();
?>