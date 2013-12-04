<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar';
$pname = 'phar://' . $fname;

$phar = new Phar($fname);

$phar['a/b'] = 'hi there';

$b = $phar['a/b'];
try {
$phar['a']->chmod(066);
} catch (Exception $e) {
echo $e->getMessage(), "\n";
}
$b->chmod(array());
lstat($pname . '/a/b'); // sets BG(CurrentLStatFile)
$b->chmod(0666);
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar'); ?>