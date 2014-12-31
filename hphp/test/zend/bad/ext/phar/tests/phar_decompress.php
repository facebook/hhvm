<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '2.phar.gz';
$pname = 'phar://' . $fname;
$file = '<?php __HALT_COMPILER(); ?>';

$files = array();
$files['a'] = 'a';
$files['b'] = 'b';
$files['c'] = 'c';

include 'files/phar_test.inc';

$phar = new Phar($fname);

$gz = $phar->compress(Phar::GZ);
copy($gz->getPath(), $fname2);
$a = new Phar($fname2);
var_dump($a->isCompressed());
$unc = $a->compress(Phar::NONE);
echo $unc->getPath() . "\n";
$unc2 = $gz->decompress();
echo $unc2->getPath() . "\n";
$unc3 = $gz->decompress('hooba.phar');
echo $unc3->getPath() . "\n";
$gz->decompress(array());
$zip = $phar->convertToData(Phar::ZIP);
ini_set('phar.readonly', 1);
try {
$gz->decompress();
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$zip->decompress();
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php error_reporting(0); ?>
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.zip');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.hooba.phar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.gz');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '2.phar.gz');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '2.phar');
?>