<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip';
$alias = 'phar://' . $fname;

$phar = new Phar($fname);
$phar->setStub("<?php
Phar::mapPhar('hio');
__HALT_COMPILER(); ?>");
$phar->addEmptyDir('a');
$phar['a/x'] = 'a';
$phar->stopBuffering();

include $fname;

echo file_get_contents($alias . '/a/x') . "\n";
var_dump(rmdir($alias . '/a'));
echo file_get_contents($alias . '/a/x') . "\n";
unlink($alias . '/a/x');
var_dump(rmdir($alias . '/a'));
?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip'); ?>