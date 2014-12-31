<?php
include dirname(__FILE__) . '/files/tarmaker.php.inc';

$fname = dirname(__FILE__) . '/tar_003.phar.tar';
$alias = 'phar://' . $fname;

$tar = new tarmaker($fname, 'none');
$tar->init();
$tar->addFile('.phar/stub.php', "<?php // tar-based phar archive stub file\n__HALT_COMPILER();");
$tar->addFile('tar_003.phpt', $g = fopen(__FILE__, 'r'));
$tar->addFile('internal/file/here', "hi there!\n");
$tar->mkDir('internal/dir');
$tar->mkDir('dir');
$tar->close();

fclose($g);

echo file_get_contents($alias . '/internal/file/here');

try {
$tar = opendir($alias . '/');
} catch (Exception $e) {
echo $e->getMessage()."\n";
}

while (false !== ($v = readdir($tar))) {
	echo (is_file($alias . '/' . $v) ? "file\n" : "dir\n");
	echo $v . "\n";
}
closedir($tar);

/* ensure none of the dir tar files were freed */
echo "second round\n";
$tar = opendir($alias . '/');
while (false !== ($v = readdir($tar))) {
	echo (is_file($alias . '/' . $v) ? "file\n" : "dir\n");
	echo $v . "\n";
}
closedir($tar);

?>
===DONE===
<?php error_reporting(0); ?>
<?php
@unlink(dirname(__FILE__) . '/tar_003.phar.tar');
?>