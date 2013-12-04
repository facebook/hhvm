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
$phar = new Phar($fname);

echo file_get_contents($pname . '/a') . "\n";
$phar->delete('a');
echo file_get_contents($pname . '/a') . "\n";
?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>