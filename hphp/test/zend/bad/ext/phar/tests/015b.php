<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = "<?php __HALT_COMPILER(); ?>";

$files = array();
$files['a'] = array('cont'=>'Hello World', 'comp'=>pack('H*', '425a6834314159265359065c89da0000009780400000400080060490002000310c082031a916c41d41e2ee48a70a1200cb913b40'),'flags'=>0x00002000);
include 'files/phar_test.inc';

var_dump(file_get_contents($pname . '/a'));
?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>