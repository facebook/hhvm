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
<?php 
unlink(dirname(__FILE__) . '/files/phar_oo_test.phar.php');
__halt_compiler();
?>