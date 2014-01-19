<?php

require_once 'files/phar_oo_test.inc';
$fname = str_replace('\\', '/', $fname);

$it = new RecursiveDirectoryIterator('phar://'.$fname);
$it = new RecursiveIteratorIterator($it);

foreach($it as $name => $ent)
{
	var_dump(str_replace(array('\\', $fname), array('/', '*'), $name));
	var_dump(str_replace(array('\\', $fname), array('/', '*'), $ent->getPathname()));
	var_dump(str_replace('\\', '/', $it->getSubPath()));
	var_dump(str_replace('\\', '/', $it->getSubPathName()));
	$sub = $it->getPathInfo();
	var_dump(str_replace('\\', '/', $sub->getFilename()));
}

?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/files/phar_oo_test.phar.php');
__halt_compiler();
?>