<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip.php';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.zip.php';
$pname = 'phar://' . $fname;
$pname2 = 'phar://' . $fname2;

$p = new Phar($fname);
$p['big'] = str_repeat(str_repeat('hi', 100), 1000);
$p['big2'] = str_repeat(str_repeat('hi', 100), 1000);

copy($fname, $fname2);
$p2 = new Phar($fname2);
var_dump(strlen($p2['big']->getContent()));
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip.php'); ?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.zip.php'); ?>