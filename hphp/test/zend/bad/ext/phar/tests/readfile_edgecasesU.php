<?php
Phar::interceptFileFuncs();
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;

chdir(dirname(__FILE__));
file_put_contents($fname, "blah\n");
file_put_contents("foob", "test\n");
readfile($fname);
unlink($fname);
mkdir($pname . '/oops');
file_put_contents($pname . '/foo/hi', '<?php
readfile("foo/" . basename(__FILE__));
$context = stream_context_create();
readfile("foob");
set_include_path("' . addslashes(dirname(__FILE__)) . '");
readfile("foob", true);
readfile("./hi", 0, $context);
readfile("../oops");
?>
');
include $pname . '/foo/hi';
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>
<?php rmdir(dirname(__FILE__) . '/poo'); ?>
<?php unlink(dirname(__FILE__) . '/foob'); ?>