<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar';
$alias = 'phar://' . $fname;
$stub = "<?php
Phar::mapPhar('hio');
__HALT_COMPILER(); ?>";

$phar = new Phar($fname);
$phar['a'] = 'a';
$phar->setStub($stub);
$phar->stopBuffering();

echo file_get_contents($alias . '/a') . "\n";
$phar->delete('a');
echo file_get_contents($alias . '/a') . "\n";

?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar'); ?>