<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$phar = new Phar($fname);
$phar->addFromString('a', 'hi');
echo file_get_contents($pname . '/a') . "\n";
$phar->addFile($pname . '/a', 'b');
echo file_get_contents($pname . '/b') . "\n";
try {
$phar->addFile($pname . '/a');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$phar->addFile($pname . '/a', 'a');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$phar->addFile(dirname(__FILE__) . '/does/not/exist');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$phar->addFile($pname . '/a', '.phar/stub.php');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$phar->addFromString('.phar/stub.php', 'hi');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>