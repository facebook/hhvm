<?php
mkdir(dirname(__FILE__) . '/testdir');
file_put_contents(dirname(__FILE__) . '/testdir/1.php', str_repeat(' ', 1455));

$phar = new Phar(dirname(__FILE__) . '/compressed.phar');
$phar->buildFromDirectory(dirname(__FILE__) . '/testdir', '/\.php$/');
$phar->setSignatureAlgorithm(Phar::SHA1);
$phar->compressFiles(Phar::GZ);
$phar->decompressFiles();

echo 'ok';
?>
<?php
if (is_file(dirname(__FILE__) . '/testdir/1.php'))
  unlink(dirname(__FILE__) . '/testdir/1.php');
if (is_dir(dirname(__FILE__) . '/testdir'))
  rmdir(dirname(__FILE__) . '/testdir');
if (is_file(dirname(__FILE__) . '/compressed.phar'))
  unlink(dirname(__FILE__) . '/compressed.phar');
?>