<?php

$pharconfig = 3;

require_once 'files/phar_oo_test.inc';

Phar::loadPhar($fname);

$pname = 'phar://' . $fname . '/a.php';

var_dump(file_get_contents($pname));
require $pname;

?>
===DONE===
<?php error_reporting(0); ?>
<?php 
unlink(dirname(__FILE__) . '/files/031.phar.php');
__halt_compiler();
?>