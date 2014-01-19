<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = "<?php __HALT_COMPILER(); ?>";

$files = array();
$files['a.php'] = '<?php echo "This is a\n"; ?>';
$files['b.php'] = '<?php echo "This is b\n"; ?>';
$files['b/c.php'] = '<?php echo "This is b/c\n"; ?>';

if (function_exists("opcache_get_status")) {
	$status = opcache_get_status();
	if ($status["opcache_enabled"]) {
		ini_set("opcache.revalidate_freq", "0");
		sleep(2);
	}
}

include 'files/phar_test.inc';

include $pname . '/a.php';
include $pname . '/b.php';
include $pname . '/b/c.php';
$md5 = md5_file($fname);
unlink($pname . '/b/c.php');
clearstatcache();
$md52 = md5_file($fname);
if ($md5 == $md52) echo 'file was not modified';
?>
===AFTER===
<?php
include 'phar://' . dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php/a.php';
include 'phar://' . dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php/b.php';
include 'phar://' . dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php/b/c.php';
?>

===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>