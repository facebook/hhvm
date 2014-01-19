<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = "<?php __HALT_COMPILER(); ?>";

$files = array();
$files['a.php'] = '<?php echo "This is a\n"; ?>';
$files['b.php'] = '<?php echo "This is b\n"; ?>';
$files['b/c.php'] = '<?php echo "This is b/c\n"; ?>';
include 'files/phar_test.inc';

$fp = fopen($pname . '/b/new.php', 'wb');
fwrite($fp, 'extra');
fclose($fp);
include $pname . '/b/c.php';
include $pname . '/b/new.php';
?>

===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>