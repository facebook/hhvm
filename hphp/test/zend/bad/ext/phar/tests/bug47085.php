<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar';

$phar = new Phar($fname, 0, 'a.phar');
$phar['x'] = 'hi';
unset($phar);
rename("phar://a.phar/x", "phar://a.phar/y");
var_dump(rename("phar://a.phar/x", "phar://a.phar/y"));
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar');?>