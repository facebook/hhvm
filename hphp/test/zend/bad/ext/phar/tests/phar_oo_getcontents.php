<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';

$phar = new Phar($fname);
$phar['a/b'] = 'file contents
this works';
$phar->addEmptyDir('hi');
echo $phar['a/b']->getContent() . "\n";
try {
echo $phar['a']->getContent(), "\n";
} catch (Exception $e) {
echo $e->getMessage(), "\n";
}
try {
echo $phar['hi']->getContent(), "\n";
} catch (Exception $e) {
echo $e->getMessage(), "\n";
}
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php');
__halt_compiler();
?>