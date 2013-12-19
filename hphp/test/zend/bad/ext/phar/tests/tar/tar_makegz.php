<?php

$fname = dirname(__FILE__) . '/tar_makegz.phar.tar';
$fname2 = dirname(__FILE__) . '/tar_makegz.phar.tar.gz';
$fname3 = dirname(__FILE__) . '/tar_makegz_b.phar.tar.gz';

$phar = new Phar($fname);
$phar['test'] = 'hi';
var_dump($phar->isFileFormat(Phar::TAR));
$phar = $phar->compress(Phar::GZ);


copy($fname2, $fname3);

$phar2 = new Phar($fname3);
var_dump($phar2->isFileFormat(Phar::TAR));
var_dump($phar2->isCompressed() == Phar::GZ);

?>
===DONE===
<?php
@unlink(dirname(__FILE__) . '/tar_makegz.phar.gz');
@unlink(dirname(__FILE__) . '/tar_makegz.phar.tar');
@unlink(dirname(__FILE__) . '/tar_makegz.phar.tar.gz');
@unlink(dirname(__FILE__) . '/tar_makegz_b.phar.tar.gz');
?>