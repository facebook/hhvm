<?php

require_once 'files/phar_oo_test.inc';

$phar = new Phar($fname);
var_dump($phar->getSignature());
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/files/phar_oo_test.phar.php');
__halt_compiler();
?>