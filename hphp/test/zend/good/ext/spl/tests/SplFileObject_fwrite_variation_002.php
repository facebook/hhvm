<?php
$file = dirname(__FILE__).'/SplFileObject_fwrite_variation_002.txt';
if(file_exists($file)) {
	unlink($file);
}
$obj = New SplFileObject($file,'w');
$obj->fwrite('test_write',12);
var_dump(file_get_contents($file));
?><?php
$file = dirname(__FILE__).'/SplFileObject_fwrite_variation_002.txt';
if(file_exists($file)) {
	unlink($file);
}
?>