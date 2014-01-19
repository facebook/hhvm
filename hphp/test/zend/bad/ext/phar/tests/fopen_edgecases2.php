<?php
Phar::interceptFileFuncs();
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;

fopen(array(), 'r');
chdir(dirname(__FILE__));
file_put_contents($fname, b"blah\n");
file_put_contents("foob", b"test\n");
$a = fopen($fname, 'rb');
echo fread($a, 1000);
fclose($a);
unlink($fname);
mkdir($pname . '/oops');
file_put_contents($pname . '/foo/hi', b'<?php
$context = stream_context_create();
$a = fopen("foob", "rb", false, $context);
echo fread($a, 1000);
fclose($a);
fopen("../oops", "r");
?>
');
include $pname . '/foo/hi';
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>
<?php rmdir(dirname(__FILE__) . '/poo'); ?>
<?php unlink(dirname(__FILE__) . '/foob'); ?>