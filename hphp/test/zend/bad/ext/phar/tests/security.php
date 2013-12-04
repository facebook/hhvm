<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.1.php';
$a = new Phar($fname);
$a->setStub('<?php
Phar::mapPhar();
$phar = new Phar(__FILE__);
var_dump($phar->isWritable());
try {
$phar["b"] = "should not work!";
} catch (Exception $e) {
echo $e->getMessage(),"\n";
}
__HALT_COMPILER();
?>');
$a['hi'] = 'hi';
unset($a);
copy($fname, $fname2);
Phar::unlinkArchive($fname);
ini_set('phar.readonly', 1);
include $fname2;
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.1.php'); ?>