<?php
$d = new DirectoryIterator('.');
var_dump($d);
var_dump(is_string($d));
preg_match('/x/', $d);
var_dump(is_string($d));
?>
===DONE===