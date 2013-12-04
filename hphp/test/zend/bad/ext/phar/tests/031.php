<?php

$pharconfig = 3;

require_once 'files/phar_oo_test.inc';

Phar::loadPhar($fname);

$pname = 'phar://' . $fname . '/a.php';

var_dump(file_get_contents($pname));
require $pname;

?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/files/phar_oo_test.phar.php');
__halt_compiler();
?>