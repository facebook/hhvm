<?php

$pharconfig = 0;

require_once 'files/phar_oo_test.inc';

try {
Phar::loadPhar($fname);
} catch (Exception $e) {
echo $e->getMessage();
}

?>
===DONE===
<?php error_reporting(0); ?>
<?php 
unlink(dirname(__FILE__) . '/files/032.phar.php');
__halt_compiler();
?>