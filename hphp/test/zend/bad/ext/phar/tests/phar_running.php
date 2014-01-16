<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;

$phar = new Phar($fname);
$phar['index.php'] = '<?php
Phar::running(array());
var_dump(Phar::running());
var_dump(Phar::running(false));
?>';
include $pname . '/index.php';
var_dump(Phar::running());
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>