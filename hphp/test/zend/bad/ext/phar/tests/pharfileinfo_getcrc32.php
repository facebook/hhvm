<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar';
$pname = 'phar://' . $fname;
$file = "<?php
Phar::mapPhar('hio');
__HALT_COMPILER(); ?>";

// compressed file length does not match incompressed lentgh for an uncompressed file

$files = array();
$files['a/subdir/here'] = array('cont'=>'a','ulen'=>1,'clen'=>1);;
include 'files/phar_test.inc';

$b = new PharFileInfo($pname . '/a/subdir');
try {
var_dump($b->getCRC32());
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}

$b = new PharFileInfo($pname . '/a/subdir/here');
try {
var_dump($b->getCRC32());
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
$a = file_get_contents($pname . '/a/subdir/here');
try {
var_dump($b->getCRC32());
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar'); ?>