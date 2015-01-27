<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar';

$phar = new Phar($fname);
$phar['long/path/name.txt'] = 'hi';
$phar->addEmptyDir('long/path');
?>
===DONE===
<?php error_reporting(0); ?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar');?>