<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = '<?php __HALT_COMPILER(); ?>';

$files = array();
$files['a'] = 'a';
$files['b'] = 'b';
$files['c'] = 'c';

include 'files/phar_test.inc';

$phar = new Phar($fname);

var_dump(file_get_contents($pname . '/a'));
var_dump($phar['a']->isCompressed());
var_dump(file_get_contents($pname . '/b'));
var_dump($phar['b']->isCompressed());
var_dump(file_get_contents($pname . '/c'));
var_dump($phar['c']->isCompressed());

$phar = new Phar($fname);
$phar->compressFiles(Phar::GZ);
var_dump(file_get_contents($pname . '/a'));
var_dump($phar['a']->isCompressed(Phar::GZ));
var_dump($phar['a']->isCompressed(Phar::BZ2));
var_dump(file_get_contents($pname . '/b'));
var_dump($phar['b']->isCompressed(Phar::GZ));
var_dump($phar['b']->isCompressed(Phar::BZ2));
var_dump(file_get_contents($pname . '/c'));
var_dump($phar['c']->isCompressed(Phar::GZ));
var_dump($phar['b']->isCompressed(Phar::BZ2));
try {
$phar->compressFiles(25);
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php');
?>