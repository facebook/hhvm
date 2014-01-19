<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://'.$fname;
$iname = '/file.txt';
$ename = '/error/..';

$p = new Phar($fname);
$p[$iname] = "foobar\n";

try
{
	$p[$ename] = "foobar\n";
}
catch(Exception $e)
{
	echo $e->getMessage() . "\n";
}

include($pname . $iname);

// extra coverage
try {
$p['.phar/oops'] = 'hi';
} catch (Exception $e) {
echo $e->getMessage(),"\n";
}
try {
$a = $p['.phar/stub.php'];
} catch (Exception $e) {
echo $e->getMessage(),"\n";
}
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>