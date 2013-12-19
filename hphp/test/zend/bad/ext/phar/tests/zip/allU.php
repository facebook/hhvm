<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip.php';
$pname = 'phar://' . $fname;
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.zip.php';
$pname2 = 'phar://' . $fname2;

$phar = new Phar($fname);

$phar->setMetadata('hi there');
$phar['a'] = 'hi';
$phar['a']->setMetadata('a meta');
$phar['b'] = 'hi2';
$phar['b']->compress(Phar::GZ);
$phar['c'] = 'hi3';
$phar['c']->compress(Phar::BZ2);
$phar['b']->chmod(0444);
$phar->setStub("<?php ok __HALT_COMPILER();");
$phar->setAlias("hime");
unset($phar);
copy($fname, $fname2);
Phar::unlinkArchive($fname);
var_dump(file_exists($fname), file_exists($pname . '/a'));

$phar = new Phar($fname2);
var_dump($phar['a']->getContent(), $phar['b']->getContent(), $phar['c']->getContent());
var_dump($phar['a']->isCompressed(), $phar['b']->isCompressed() == Phar::GZ, $phar['c']->isCompressed() == Phar::BZ2);
var_dump((string) decoct(fileperms($pname2 . '/b')));
var_dump($phar->getStub());
var_dump($phar->getAlias());
var_dump($phar->getMetadata());
var_dump($phar['a']->getMetadata());
?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.zip.php'); ?>