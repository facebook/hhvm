<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = b"<?php __HALT_COMPILER(); ?>";
// file length is too short

$files = array();
// "hi" gzdeflated
$files['a'] = array('cont'=>b'a','comp'=> (binary)pack('H*', 'cbc80400'),'flags'=>0x00001000, 'ulen' => 1, 'clen' => 4);
$files['b'] = $files['a'];
$files['c'] = array('cont'=>b'*');
$files['d'] = $files['a'];
include 'files/phar_test.inc';

var_dump(file_get_contents($pname . '/a'));
var_dump(file_get_contents($pname . '/b'));
var_dump(file_get_contents($pname . '/c'));
var_dump(file_get_contents($pname . '/d'));
?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>