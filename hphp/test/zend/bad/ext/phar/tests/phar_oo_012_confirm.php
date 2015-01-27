<?php

$pharconfig = 0;

require_once 'files/phar_oo_test.inc';

$phar = new Phar($fname);
$phar->setInfoClass('SplFileObject');

$phar['f.php'] = 'hi';
var_dump(isset($phar['f.php']));
echo $phar['f.php'];
echo "\n";
$md5 = md5_file($fname);
unset($phar['f.php']);
$md52 = md5_file($fname);
if ($md5 == $md52) echo 'File on disk has not changed';
var_dump(isset($phar['f.php']));

?>
===DONE===
<?php error_reporting(0); ?>
<?php 
unlink(dirname(__FILE__) . '/files/phar_oo_012_confirm.phar.php');
__halt_compiler();
?>