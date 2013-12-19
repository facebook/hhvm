<?php
chdir(dirname(__FILE__));
try {
	$p = new Phar('brandnewphar.phar');
	$p['file1.txt'] = 'hi';
	var_dump(strlen($p->getStub()));
	$p->setStub("<?php
function __autoload(\$class)
{
    include 'phar://' . str_replace('_', '/', \$class);
}
Phar::mapPhar('brandnewphar.phar');
include 'phar://brandnewphar.phar/startup.php';
__HALT_COMPILER();
?>");
	var_dump($p->getStub());
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/brandnewphar.phar');
?>