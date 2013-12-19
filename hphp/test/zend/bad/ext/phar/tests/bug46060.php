<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.tar';

$phar = new PharData($fname);
$phar->addEmptyDir('blah/');
$phar->addFromString('test/', '');

copy($fname, $fname2);
$phar = new PharData($fname2);

var_dump($phar['blah']->isDir(), $phar['test']->isDir());
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.tar');
__HALT_COMPILER();
?>