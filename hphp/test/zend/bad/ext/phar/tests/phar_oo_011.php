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
<?php error_reporting(0); ?>
<?php 
unlink(dirname(__FILE__) . '/files/phar_oo_011.phar.php');
__halt_compiler();
?>