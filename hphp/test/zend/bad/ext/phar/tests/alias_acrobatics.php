<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar';

$p = new Phar($fname);

$p->setAlias('foo');
$p['unused'] = 'hi';
try {
$a = new Phar($fname2, 0, 'foo');
} catch (Exception $e) {
echo $e->getMessage(),"\n";
}
copy($fname, $fname2);
echo "2\n";
try {
$a = new Phar($fname2);
} catch (Exception $e) {
echo $e->getMessage(),"\n";
}
try {
$b = new Phar($fname, 0, 'another');
} catch (Exception $e) {
echo $e->getMessage(),"\n";
}
?>
===DONE===
<?php error_reporting(0); ?>
<?php
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar');
?>