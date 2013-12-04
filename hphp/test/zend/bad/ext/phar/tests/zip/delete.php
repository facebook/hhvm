<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip';
$alias = 'phar://' . $fname;
$file = "<?php
Phar::mapPhar('hio');
__HALT_COMPILER(); ?>";

$phar = new Phar($fname);
$phar['a'] = 'a';
$phar->setStub($file);
$phar->stopBuffering();

echo file_get_contents($alias . '/a') . "\n";
$phar->delete('a');
echo file_get_contents($alias . '/a') . "\n";
?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip'); ?>