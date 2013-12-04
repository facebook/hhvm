<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip';
$alias = 'phar://' . $fname;

$phar = new Phar($fname);
$phar->setStub("<?php
Phar::mapPhar('hio');
__HALT_COMPILER(); ?>");
$phar['a/x'] = 'a';
$phar->stopBuffering();

include $fname;

echo file_get_contents($alias . '/a/x') . "\n";
rename($alias . '/a', $alias . '/b');
echo file_get_contents($alias . '/b/x') . "\n";
echo file_get_contents($alias . '/a/x') . "\n";
?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip'); ?>