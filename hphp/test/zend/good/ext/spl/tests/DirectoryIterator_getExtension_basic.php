<?hh
<<__EntryPoint>> function main(): void {
$dir = sys_get_temp_dir().'/'.'DirectoryIterator::getExtension' . DIRECTORY_SEPARATOR;
mkdir($dir);

$files = vec['test.txt', 'test.extension', 'test..', 'test.', 'test'];
foreach ($files as $file) {
    touch($dir . $file);
}

$dit_exts = vec[];
$nfo_exts = vec[];
$skip = vec['.', '..'];

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

foreach ($files as $file) {
    unlink($dir . $file);
}
rmdir($dir);
}
