<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = "<?php __HALT_COMPILER(); ?>";
// file length is too short

$files = array();
$files['a'] = array('cont'=>'a','flags'=>0x00001000, 'clen' => 1);
include 'files/phar_test.inc';

echo file_get_contents($pname . '/a');
?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>