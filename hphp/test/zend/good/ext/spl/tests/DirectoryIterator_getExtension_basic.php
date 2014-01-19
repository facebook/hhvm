<?php
$dir = __DIR__ . DIRECTORY_SEPARATOR . md5('DirectoryIterator::getExtension') . DIRECTORY_SEPARATOR;
mkdir($dir);

$files = array('test.txt', 'test.extension', 'test..', 'test.', 'test');
foreach ($files as $file) {
    touch($dir . $file);
}

$dit_exts = array();
$nfo_exts = array();
$skip = array('.', '..');

foreach (new DirectoryIterator($dir) as $file) {
    if (in_array($file->getFilename(), $skip)) {
        continue;
    }
    $dit_exts[] = $file->getExtension();
    $nfo_exts[] = pathinfo($file->getFilename(), PATHINFO_EXTENSION);
}
var_dump($dit_exts === $nfo_exts);
sort($dit_exts);
var_dump($dit_exts);
?>
<?php
$dir   = __DIR__ . DIRECTORY_SEPARATOR . md5('DirectoryIterator::getExtension') . DIRECTORY_SEPARATOR;
$files = array('test.txt', 'test.extension', 'test..', 'test.', 'test');
foreach ($files as $file) {
    unlink($dir . $file);
}
rmdir($dir);
?>