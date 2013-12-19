<?php
include dirname(__FILE__) . '/files/tarmaker.php.inc';
$fname = dirname(__FILE__) . '/tar_gzip.phar';
$pname = 'phar://' . $fname;
$fname2 = dirname(__FILE__) . '/tar_gzip.phar.tar';
$pname2 = 'phar://' . $fname2;

$a = new tarmaker($fname, 'zlib');
$a->init();
$a->addFile('tar_004.php', '<?php var_dump(__FILE__);');
$a->addFile('internal/file/here', "hi there!\n");
$a->mkDir('internal/dir');
$a->mkDir('dir');
$a->addFile('.phar/stub.php', '<?php
Phar::mapPhar();
var_dump("it worked");
include "phar://" . __FILE__ . "/tar_004.php";
');
$a->close();

include $fname;

$a = new Phar($fname);
$a['test'] = 'hi';
copy($fname, $fname2);
$b = new Phar($fname2);
var_dump($b->isFileFormat(Phar::TAR));
var_dump($b->isCompressed() == Phar::GZ);
?>
===DONE===
<?php
@unlink(dirname(__FILE__) . '/tar_gzip.phar');
@unlink(dirname(__FILE__) . '/tar_gzip.phar.tar');
?>