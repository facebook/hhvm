<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = "<?php
Phar::mapPhar('hio');
__HALT_COMPILER(); ?>";

$files = array();
$files['a/x'] = 'a';
include 'files/phar_test.inc';
include $fname;

echo file_get_contents($pname . '/a/x') . "\n";
rename($pname . '/a', $pname . '/b');
echo file_get_contents($pname . '/b/x') . "\n";
echo file_get_contents($pname . '/a/x') . "\n";
?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>