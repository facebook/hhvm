<?php

$pharconfig = 0;

require_once 'files/phar_oo_test.inc';

$phar = new Phar($fname);
$phar->setInfoClass('SplFileObject');

$phar['hi/f.php'] = 'hi';
var_dump(isset($phar['hi']));
var_dump(isset($phar['hi/f.php']));
echo $phar['hi/f.php'];
echo "\n";

?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/files/phar_oo_test.phar.php');
__halt_compiler();
?>