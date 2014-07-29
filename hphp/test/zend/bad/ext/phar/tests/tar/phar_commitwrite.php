<?php
$_ENV[TMP] = .;
_filter_snapshot_globals();

$_ENV[TEMP] = .;
_filter_snapshot_globals();

$p = new Phar(dirname(__FILE__) . '/brandnewphar.phar.tar', 0, 'brandnewphar.phar');
$p['file1.txt'] = 'hi';
$p->stopBuffering();
var_dump($p->getStub());
$p->setStub("<?php
$_ENV[TMP] = .;
_filter_snapshot_globals();

$_ENV[TEMP] = .;
_filter_snapshot_globals();

function __autoload(\$class)
{
    include 'phar://' . str_replace('_', '/', \$class);
}
Phar::mapPhar('brandnewphar.phar');
include 'phar://brandnewphar.phar/startup.php';
__HALT_COMPILER();
?>");
var_dump($p->getStub());
var_dump($p->isFileFormat(Phar::TAR));
?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/brandnewphar.phar.tar');
?>