<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = "<?php
Phar::mapPhar('hio');
__HALT_COMPILER(); ?>";

$files = array();
$files['a'] = 'a';
include 'files/phar_test.inc';
include $fname;

echo file_get_contents($pname . '/a') . "\n";
rename($pname . '/a', $pname . '/b');
echo file_get_contents($pname . '/b') . "\n";
echo file_get_contents($pname . '/a') . "\n";
?>
<?php error_reporting(0); ?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>