<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';

$phar = new Phar($fname);
$phar['top.txt'] = 'hi';
$phar['sub/top.txt'] = 'there';
$phar['another.file.txt'] = 'wowee';
$newphar = new Phar($fname);
foreach (new RecursiveIteratorIterator($newphar) as $path => $obj) {
	var_dump($obj->getPathName());
}
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php');
__halt_compiler();
?>