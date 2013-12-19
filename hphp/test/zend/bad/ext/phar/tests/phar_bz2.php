<?php
$fname = dirname(__FILE__) . '/phar_bz2.phar';
$pname = 'phar://' . $fname;
$fname2 = dirname(__FILE__) . '/phar_bz2.2.phar';
$pname2 = 'phar://' . $fname2;

$file = '<?php
Phar::mapPhar();
var_dump("it worked");
include "phar://" . __FILE__ . "/tar_004.php";
__HALT_COMPILER();';

$files = array();
$files['tar_004.php']   = '<?php var_dump(__FILE__);';
$files['internal/file/here']   = "hi there!\n";
$files['internal/dir/'] = '';
$files['dir/'] = '';
$bz2 = true;

include 'files/phar_test.inc';

include $fname;

$a = new Phar($fname);
$a['test'] = 'hi';
copy($fname, $fname2);
$a->setAlias('another');
$b = new Phar($fname2);
var_dump($b->isFileFormat(Phar::PHAR));
var_dump($b->isCompressed() == Phar::BZ2);
// additional code coverage
$b->isFileFormat(array());
try {
$b->isFileFormat(25);
} catch (Exception $e) {
echo $e->getMessage(),"\n";
}
?>
===DONE===
<?php
@unlink(dirname(__FILE__) . '/phar_bz2.phar');
@unlink(dirname(__FILE__) . '/phar_bz2.2.phar');
?>