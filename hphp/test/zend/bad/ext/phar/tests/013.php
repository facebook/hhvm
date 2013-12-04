<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = "<?php __HALT_COMPILER(); ?>";
// filesize should be 1, and is 2

$files = array();
$files['a'] = array('cont'=>'a', 'ulen'=>2, 'clen'=>2);
include 'files/phar_test.inc';

echo file_get_contents($pname.'/a');
?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>