<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = "<?php __HALT_COMPILER(); ?>";

$files = array();
$files['a'] = array('cont'=>'a','comp'=>chr(75) . chr(4) . chr(0) /* 'a' gzdeflated */, 'flags'=>0x00001000);
include 'files/phar_test.inc';

echo file_get_contents($pname .'/a');
?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>