<?php

$fname = dirname(__FILE__) . '/tar_makebz2.phar.tar';
$fname2 = dirname(__FILE__) . '/tar_makebz2.phar.tar.bz2';
$fname3 = dirname(__FILE__) . '/tar_makebz2_b.phar.tar.bz2';

$phar = new Phar($fname);
$phar['test'] = 'hi';
var_dump($phar->isFileFormat(Phar::TAR));
$phar = $phar->compress(Phar::BZ2);

copy($fname2, $fname3);

$phar2 = new Phar($fname3);
var_dump($phar2->isFileFormat(Phar::TAR));
var_dump($phar2->isCompressed() == Phar::BZ2);

?>
===DONE===
<?php
@unlink(dirname(__FILE__) . '/tar_makebz2.phar.bz2');
@unlink(dirname(__FILE__) . '/tar_makebz2.phar.tar');
@unlink(dirname(__FILE__) . '/tar_makebz2.phar.tar.bz2');
@unlink(dirname(__FILE__) . '/tar_makebz2_b.phar.tar.bz2');
?>