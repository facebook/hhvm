<?php
Phar::interceptFileFuncs();
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.php';
$pname = 'phar://' . $fname;
file_put_contents($fname2, '<?php Phar::unlinkArchive("' . addslashes($fname) . '");');
file_put_contents($pname . '/foo/hi', '<?php
include "' . addslashes($fname2) . '";
readfile("foo/hi");
fopen("foo/hi", "r");
echo file_get_contents("foo/hi");
var_dump(is_file("foo/hi"),is_link("foo/hi"),is_dir("foo/hi"),file_exists("foo/hi"),stat("foo/hi"));
opendir("foo/hi");
?>
');
include $pname . '/foo/hi';
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.php'); ?>