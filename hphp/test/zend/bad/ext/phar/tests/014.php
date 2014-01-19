<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = "<?php __HALT_COMPILER(); ?>";
// wrong crc32

$files = array();
$files['a'] = array('cont'=>'a', 'crc32'=>crc32(b'aX'));
include 'files/phar_test.inc';

echo file_get_contents($pname.'/a');
?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>