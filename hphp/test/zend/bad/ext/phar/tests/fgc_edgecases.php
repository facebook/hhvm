<?php

Phar::interceptFileFuncs();

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;

file_get_contents(array());
chdir(dirname(__FILE__));
file_put_contents($fname, "blah\n");
file_put_contents("foob", "test\n");
echo file_get_contents($fname);
unlink($fname);
mkdir($pname . '/oops');

file_put_contents($pname . '/foo/hi', '<?php
echo file_get_contents("foo/" . basename(__FILE__));
$context = stream_context_create();
file_get_contents("./hi", 0, $context, 0, -1);
echo file_get_contents("foob");
set_include_path("' . addslashes(dirname(__FILE__)) . '");
echo file_get_contents("foob", true);
echo file_get_contents("./hi", 0, $context);
echo file_get_contents("../oops");
echo file_get_contents("./hi", 0, $context, 50000);
echo file_get_contents("./hi");
echo file_get_contents("./hi", 0, $context, 0, 0);
?>
');

include $pname . '/foo/hi';

?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>
<?php rmdir(dirname(__FILE__) . '/poo'); ?>
<?php unlink(dirname(__FILE__) . '/foob'); ?>