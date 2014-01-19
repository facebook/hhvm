<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
Phar::interceptFileFuncs();
mkdir('phar://');
mkdir('phar://foo.phar');
$a = new Phar($fname);
$a['a'] = 'hi';
mkdir($pname . '/a');
rmdir('phar://');
rmdir('phar://foo.phar');
rmdir($pname . '/a');
$a->addEmptyDir('bb');
$a->addEmptyDir('bb');
try {
$a->addEmptyDir('.phar');
} catch (Exception $e) {
echo $e->getMessage(),"\n";
}
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php');
__HALT_COMPILER();
?>