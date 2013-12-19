<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = "<?php __HALT_COMPILER(); ?>";

$files = array();
$files['a'] = '<?php echo "This is a\n"; ?>';
$files['b'] = '<?php echo "This is b\n"; ?>';
$files['b/b'] = '<?php echo "This is b/b\n"; ?>';

include 'files/phar_test.inc';

include $pname . '/a';
include $pname . '/b';
include $pname . '/b/b';

?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>