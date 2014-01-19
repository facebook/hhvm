<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar';
$pname = 'phar://' . $fname;

$phar = new Phar($fname);

$phar['a/b'] = 'hi there';
$tar = $phar->convertToData(Phar::TAR);

$b = $phar['a/b'];
try {
$phar['a']->setMetadata('hi');
} catch (Exception $e) {
echo $e->getMessage(), "\n";
}
try {
$phar['a']->delMetadata();
} catch (Exception $e) {
echo $e->getMessage(), "\n";
}
ini_set('phar.readonly', 1);
try {
$b->setMetadata('hi');
} catch (Exception $e) {
echo $e->getMessage(), "\n";
}
try {
$b->delMetadata();
} catch (Exception $e) {
echo $e->getMessage(), "\n";
}
ini_set('phar.readonly', 0);
$b->setMetadata(1,2,3);
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar'); ?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar'); ?>