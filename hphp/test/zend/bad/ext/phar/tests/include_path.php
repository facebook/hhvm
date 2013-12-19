<?php
$fname = dirname(__FILE__) . '/tempmanifest1.phar.php';
$a = new Phar($fname);
$a['file1.php'] = 'file1.php
';
$a['test/file1.php'] = 'test/file1.php
';
unset($a);
set_include_path('.' . PATH_SEPARATOR . 'phar://' . $fname);
include 'file1.php';
set_include_path('.' . PATH_SEPARATOR . 'phar://' . $fname . '/test');
include 'file1.php';
include 'file2.php';
?>
===DONE===
<?php
@unlink(dirname(__FILE__) . '/tempmanifest1.phar.php');
?>