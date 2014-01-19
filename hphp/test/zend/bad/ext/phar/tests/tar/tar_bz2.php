<?php
include dirname(__FILE__) . '/files/tarmaker.php.inc';

$fname = dirname(__FILE__) . '/tar_bz2.phar';
$alias = 'phar://' . $fname;
$fname2 = dirname(__FILE__) . '/tar_bz2.phar.tar';
$alias2 = 'phar://' . $fname2;

$tar = new tarmaker($fname, 'bz2');
$tar->init();
$tar->addFile('tar_004.php', '<?php var_dump(__FILE__);');
$tar->addFile('internal/file/here', "hi there!\n");
$tar->mkDir('internal/dir');
$tar->mkDir('dir');
$tar->addFile('.phar/stub.php', '<?php
var_dump(__FILE__);
var_dump(substr(__FILE__, 0, 4) != "phar");
Phar::mapPhar();
var_dump("it worked");
include "phar://" . __FILE__ . "/tar_004.php";
__HALT_COMPILER();
');
$tar->close();

include $alias;

$phar = new Phar($fname);
$phar['test'] = 'hi';

copy($fname, $fname2);

$phar2 = new Phar($fname2);
var_dump($phar2->isFileFormat(Phar::TAR));
var_dump($phar2->isCompressed() == Phar::BZ2);

?>
===DONE===
<?php
@unlink(dirname(__FILE__) . '/tar_bz2.phar');
@unlink(dirname(__FILE__) . '/tar_bz2.phar.tar');
?>