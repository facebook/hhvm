<?hh <<__EntryPoint>> function main(): void {
$dir = __DIR__ . DIRECTORY_SEPARATOR . md5('DirectoryIterator::getExtension') . DIRECTORY_SEPARATOR;
mkdir($dir);

$files = varray['test.txt', 'test.extension', 'test..', 'test.', 'test'];
foreach ($files as $file) {
    touch($dir . $file);
}

$dit_exts = varray[];
$nfo_exts = varray[];
$skip = varray['.', '..'];

foreach (new DirectoryIterator($dir) as $file) {
    if (in_array($file->getFilename(), $skip)) {
        continue;
    }
    $dit_exts[] = $file->getExtension();
    $nfo_exts[] = pathinfo($file->getFilename(), PATHINFO_EXTENSION);
}
var_dump($dit_exts === $nfo_exts);
sort(inout $dit_exts);
var_dump($dit_exts);
error_reporting(0);
$dir   = __DIR__ . DIRECTORY_SEPARATOR . md5('DirectoryIterator::getExtension') . DIRECTORY_SEPARATOR;
$files = varray['test.txt', 'test.extension', 'test..', 'test.', 'test'];
foreach ($files as $file) {
    unlink($dir . $file);
}
rmdir($dir);
}
