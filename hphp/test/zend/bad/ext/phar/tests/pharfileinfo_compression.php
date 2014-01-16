<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar';
$pname = 'phar://' . $fname;

$phar = new Phar($fname);

$phar['a/b'] = 'hi there';

$b = $phar['a/b'];

$b->isCompressed(array());
try {
$b->isCompressed(25);
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$b->compress(25);
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
$tar = $phar->convertToData(Phar::TAR);

$c = $tar['a/b'];
try {
$c->compress(Phar::GZ);
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$phar['a']->compress(Phar::GZ);
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
ini_set('phar.readonly', 1);
try {
$b->compress(Phar::GZ);
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
ini_set('phar.readonly', 0);
var_dump($b->compress(Phar::GZ));
var_dump($b->compress(Phar::GZ));
var_dump($b->compress(Phar::BZ2));
var_dump($b->compress(Phar::BZ2));

echo "decompress\n";

ini_set('phar.readonly', 1);
try {
$phar['a/b']->decompress();
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
ini_set('phar.readonly', 0);
try {
$phar['a']->decompress();
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
var_dump($b->decompress());
var_dump($b->decompress());

?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar'); ?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar'); ?>