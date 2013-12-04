<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = "<?php
Phar::mapPhar('hio');
__HALT_COMPILER(); ?>";

// compressed file length does not match incompressed lentgh for an uncompressed file

$files = array();
$files['a'] = array('cont'=>'a','ulen'=>1,'clen'=>2);;
include 'files/phar_test.inc';
try {
include $fname;
echo file_get_contents('phar://hio/a');
} catch (Exception $e) {
echo $e->getMessage();
}
?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>