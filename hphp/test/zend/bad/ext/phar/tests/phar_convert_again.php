<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '2.tbz';
$pname = 'phar://' . $fname;
$stub = '<?php echo "first stub\n"; __HALT_COMPILER(); ?>';
$file = $stub;

$files = array();
$files['a'] = 'a';
$files['b'] = 'b';
$files['c'] = 'c';

include 'files/phar_test.inc';

$phar = new Phar($fname);
$zip = $phar->convertToData(Phar::ZIP);
echo $zip->getPath() . "\n";
$tgz = $phar->convertToData(Phar::TAR, Phar::GZ);
echo $tgz->getPath() . "\n";
$tbz = $phar->convertToData(Phar::TAR, Phar::BZ2);
echo $tbz->getPath() . "\n";
try {
$phar = $tbz->convertToExecutable(Phar::PHAR, Phar::NONE);
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
copy($tbz->getPath(), $fname2);
$tbz = new PharData($fname2);
$phar = $tbz->convertToExecutable(Phar::PHAR, Phar::NONE);
echo $phar->getPath() . "\n";
$phar['a'] = 'hi';
$phar['a']->setMetadata('hi');
$zip = $phar->convertToExecutable(Phar::ZIP);
echo $zip->getPath() . "\n";
echo $zip['a']->getMetadata() . "\n";
$data = $zip->convertToData();
echo $data->getPath() . "\n";
// extra code coverage
try {
$data->setStub('hi');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$data->setDefaultStub();
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$data->setAlias('hi');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
$tar = $phar->convertToExecutable(Phar::TAR);
echo $tar->getPath() . "\n";
$data = $tar->convertToData();
echo $data->getPath() . "\n";
$tgz = $tar->convertToExecutable(null, Phar::GZ);
echo $tgz->getPath() . "\n";
try {
$tgz->convertToExecutable(25);
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$tgz->convertToExecutable(Phar::ZIP, Phar::GZ);
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$tgz->convertToExecutable(Phar::ZIP, Phar::BZ2);
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$phar->convertToData();
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$tgz->convertToData(Phar::PHAR);
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$tgz->convertToData(25);
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$tgz->convertToData(Phar::ZIP, Phar::GZ);
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$tgz->convertToData(Phar::ZIP, Phar::BZ2);
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$tgz->convertToExecutable(Phar::TAR, 25);
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$tgz->convertToData(Phar::TAR, 25);
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
// extra code coverage
try {
$data->setStub('hi');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$data->setAlias('hi');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$data->setDefaultStub();
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
try {
$tgz->convertToData(Phar::TAR, Phar::GZ, '.phar.tgz.oops');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}

try {
$phar->convertToExecutable(Phar::TAR, Phar::GZ, '.tgz.oops');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}

try {
$tgz->convertToData(Phar::TAR, Phar::GZ, '.phar/.tgz.oops');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
?>
===DONE===
<?php
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar.gz');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.zip');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar.gz');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar.bz2');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '2.tbz');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '2.phar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '2.phar.tar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '2.tar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '2.phar.zip');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '2.zip');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '2.phar.tar.gz');
__HALT_COMPILER();
?>