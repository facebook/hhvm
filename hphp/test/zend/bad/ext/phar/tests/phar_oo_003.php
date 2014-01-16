<?php

require_once 'files/phar_oo_test.inc';

$phar = new Phar($fname);
$phar->setInfoClass();
foreach($phar as $name => $ent)
{
	var_dump($ent->getFilename());
	if ($ent->isDir()) {
		var_dump('DIR');
	} else {
		var_dump($ent->openFile()->fgets());
		include $ent->getPathName();
	}
}

?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/files/phar_oo_test.phar.php');
__halt_compiler();
?>